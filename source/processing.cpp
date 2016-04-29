#include "processing.hpp"

#include <iostream>
#include <regex>
#include <iterator>
#include <sstream>
#include <valarray>
//#include <boost/filesystem.hpp>
//#include "boost/filesystem/fstream.hpp"
//#include <boost/algorithm/string/join.hpp>
//#include <boost/regex.hpp>
#include <omp.h>

//namespace fs = boost::filesystem;
namespace processing {
	
	typedef map<pair<int, int>, string> basedistri2d;


/****************** Error Estimation **************************************************/

    void countMutationFrequency(int barcode, int pos1, int pos2, int nucl1, int nucl2, utils::rateArray& countsPP,
                                utils::WeightPerPosPair& weightPos, std::map<int, utils::WeightPerPosPair>& weightsPerSample,
                                utils::RatesPerPosPair& expRatesFalseDetect,  std::map<int, utils::RatesPerPosPair>& expFalseDetectPerSample) {
        utils::Values counts(countsPP);

        int numWtWt = counts[(nucl1-1)*4+nucl2-1];

        int seqSum = counts.sum();

        //count all sequences for a pair combination
        weightPos.add(pos1, pos2, seqSum);
        weightsPerSample[barcode].add(pos1, pos2, seqSum);
        counts.divide(numWtWt);

        expRatesFalseDetect.add(pos1, pos2, counts);

        expFalseDetectPerSample[barcode].add(pos1, pos2, counts);

    }

    void estimateError(const string& expDir, utils::Parameter& param, utils::DataContainer& data) {
		std::cout << "Estimate Error...." << std::endl;

        //clear data before computing
        data.clearErrors();

		utils::sampleContainer::iterator boundIt, unboundIt;
        int numberOfExp = 0;
		//Count number of wt samples for each pair of positions
		utils::WeightPerPosPair weightPosBound;
		utils::WeightPerPosPair weightPosUnbound;		
		
		utils::RatesPerPosPair expRatesFalseDetectBound;
		utils::RatesPerPosPair expRatesFalseDetectUnbound;
		
		//for the compariso of each sample (error plot and variation coefficient)		
		std::map<int, utils::WeightPerPosPair> weightsPerSampleBound;
		std::map<int, utils::WeightPerPosPair> weightsPerSampleUnbound;
		
		std::map<int, utils::RatesPerPosPair> expFalseDetectPerSampleBound;
		std::map<int, utils::RatesPerPosPair> expFalseDetectPerSampleUnbound;


		//for each experiment pair with wildtype(library=0)
        for(boundIt=data.bound.begin(), unboundIt=data.unbound.begin(); boundIt != data.bound.end(); ++boundIt, ++unboundIt) {

            //for errors only exermine wt samples (lib = 0)
            if((*boundIt).library == 0) {
                ++numberOfExp;
                int boundBarcode = (*boundIt).barcode;
                int unboundBarcode = (*unboundIt).barcode;

                utils::countsPerPosPair boundCountsPP;
                utils::countsPerPosPair unboundCountsPP;
                ioTools::readExperimentFile(boundBarcode, expDir+"/2d", boundCountsPP);
                ioTools::readExperimentFile(unboundBarcode, expDir+"/2d", unboundCountsPP);
                //hier nicht parallelisierbar wegen iterator... mögliche lösungen: 2 for schleifen (pos1, pos2) oder __gnu_parallel for each?
//                #pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, boundBarcode, unboundBarcode, param, data, weightPosBound, weightPosUnbound, weightPosTotal, expRatesFalseDetectBound, expRatesFalseDetectUnbound, expRatesFalseDetectTotal, boundCountsPP, unboundCountsPP)
//                for(utils::countsPerPosPair::iterator pos1PairIt = boundCountsPP.begin(); posPairIt != boundCountsPP.end(); ++posPairIt) {
                for(int pos1 = param.seqBegin; pos1 <= param.seqEnd; ++pos1) {
                    int pos2 = 1;
                    for(auto boundPos2It = boundCountsPP[pos1].begin(); boundPos2It != boundCountsPP[pos1].end() && pos2 <= param.seqEnd; ++boundPos2It) {

                          pos2 = boundPos2It->first;

                        //only consider data in the given interval
                        if(pos1 >= param.seqBegin && pos1 <= param.seqEnd && pos2 >= param.seqBegin && pos2 <= param.seqEnd) {
                            int nucl1 = data.ref[pos1];
                            int nucl2 = data.ref[pos2];
                             /**** Auslagern ****/
                            countMutationFrequency(boundBarcode, pos1, pos2, nucl1, nucl2, boundPos2It->second, weightPosBound, weightsPerSampleBound,
                                                   expRatesFalseDetectBound, expFalseDetectPerSampleBound);
                            countMutationFrequency(unboundBarcode, pos1, pos2, nucl1, nucl2, unboundCountsPP[pos1][pos2], weightPosUnbound, weightsPerSampleUnbound,
                                                  expRatesFalseDetectUnbound, expFalseDetectPerSampleUnbound);

                        }
                    }
                }
                weightsPerSampleBound[boundBarcode].normalize();
                weightsPerSampleUnbound[unboundBarcode].normalize();
            }
		}
		
		// average Rate of all experiments
		expRatesFalseDetectBound.divide(numberOfExp);
		expRatesFalseDetectUnbound.divide(numberOfExp);
		
		//normalize by maximamum value for position 1
		weightPosBound.normalize();
		weightPosUnbound.normalize();
		
		int actualPos1 = param.seqBegin;
		std::map<int, std::multiset<double>> sumsPerSampleBound;
		std::map<int, std::multiset<double>> sumsPerSampleUnbound;

		std::vector<std::multiset<double>> sums_perBaseBound(3);
		std::vector<std::multiset<double>> sums_perBaseUnbound(3);
        //for join errors
		std::map<int, std::vector<std::multiset<double>>> sumsPerSample_perBaseBound;
		std::map<int, std::vector<std::multiset<double>>> sumsPerSample_perBaseUnbound;

		std::vector<double> totalSum_perBase(3);

        for(utils::RatesPerPosPair::iterator it = expRatesFalseDetectBound.begin(); it != expRatesFalseDetectBound.end(); ++it) {
            int pos1 = expRatesFalseDetectBound.getFirstPos(it);
            int pos2 = expRatesFalseDetectBound.getSecondPos(it);
			int wtBase1 = data.ref[pos1];
			int wtBase2 = data.ref[pos2];
			
			if(pos1 != actualPos1) {
				//save collected values for last position, refresh the containers for next position
                if(!(totalSum_perBase[0] == 0 && totalSum_perBase[1] == 0 && totalSum_perBase[2] == 0)) {
					std::vector<double> mediansBound(3);
					std::vector<double> mediansUnbound(3);
					for(int i = 0; i < 3; ++i) {
                        if(sums_perBaseBound[i].size() > 0 && sums_perBaseUnbound[i].size() > 0)
                        {
                            mediansBound[i] = utils::getMedian(sums_perBaseBound[i]);
                            mediansUnbound[i] = utils::getMedian(sums_perBaseUnbound[i]);
                        }
									
						totalSum_perBase[i] = 0;
                        sums_perBaseBound[i].clear();
                        sums_perBaseUnbound[i].clear();
					}
					data.medianExpKappaBound_perBase[actualPos1] = mediansBound;
					data.medianExpKappaUnbound_perBase[actualPos1] = mediansUnbound;
					
					// total error rate for each sample
					for(auto boundIt = weightsPerSampleBound.begin(), unboundIt = weightsPerSampleUnbound.begin();  boundIt != weightsPerSampleBound.end(); ++boundIt, ++unboundIt) {

						int boundBarcode = boundIt->first;
						int unboundBarcode = unboundIt->first;
                        if(!(sumsPerSampleBound[boundBarcode]).empty() && !(sumsPerSampleUnbound[unboundBarcode]).empty())
                        {
                            data.medianExpKappaTotal_perSampleBound[boundBarcode][actualPos1] = utils::getMedian(sumsPerSampleBound[boundBarcode]);
                            data.medianExpKappaTotal_perSampleUnbound[unboundBarcode][actualPos1] = utils::getMedian(sumsPerSampleUnbound[unboundBarcode]);

                            data.perc25ExpKappaTotal_perSampleBound[boundBarcode][actualPos1] = utils::getPercentile(sumsPerSampleBound[boundBarcode], 25);
                            data.perc25ExpKappaTotal_perSampleUnbound[unboundBarcode][actualPos1] = utils::getPercentile(sumsPerSampleUnbound[unboundBarcode], 25);
                            data.perc75ExpKappaTotal_perSampleBound[boundBarcode][actualPos1] = utils::getPercentile(sumsPerSampleBound[boundBarcode], 75);
                            data.perc75ExpKappaTotal_perSampleUnbound[unboundBarcode][actualPos1] = utils::getPercentile(sumsPerSampleUnbound[unboundBarcode], 75);
                        }

                        (data.medianExpKappaTotal_perBase_perSampleBound[boundBarcode][actualPos1]).resize(3);

                        (data.medianExpKappaTotal_perBase_perSampleUnbound[boundBarcode][actualPos1]).resize(3);
                        std::vector<double> mediansBound(3);
                        std::vector<double> mediansUnbound(3);
                        for(int i= 0; i < 3; ++i)
                        {
                            if(!(sumsPerSample_perBaseBound[boundBarcode][i]).empty() && !(sumsPerSample_perBaseUnbound[unboundBarcode][i]).empty())
                            {
                                mediansBound[i] = utils::getMedian(sumsPerSample_perBaseBound[boundBarcode][i]);
                                mediansUnbound[i] = utils::getMedian(sumsPerSample_perBaseUnbound[unboundBarcode][i]);
                            }
                            sumsPerSample_perBaseBound[boundBarcode][i].clear();
                            sumsPerSample_perBaseUnbound[unboundBarcode][i].clear();
                        }
                        data.medianExpKappaTotal_perBase_perSampleBound[boundBarcode][actualPos1] = mediansBound;
                        data.medianExpKappaTotal_perBase_perSampleUnbound[unboundBarcode][actualPos1] = mediansUnbound;
					}
				} 
				actualPos1 = pos1;
			}
			
			
            if(weightPosBound[std::make_pair(pos1, pos2)] > param.weightThreshold) {
				utils::rateArray countvalues = expRatesFalseDetectBound.getValues(pos1, pos2);
				// save positions per pair, where nucl1 is mutant and nucl2 is wt
				for(int mnucl=1, i=0; mnucl<5; ++mnucl) {					
					if(wtBase1 != mnucl) {
						int idx = 4*(mnucl-1)+wtBase2-1;
						if(countvalues[idx] > 0) {
							sums_perBaseBound[i].insert(countvalues[idx]);
                            totalSum_perBase[i] += countvalues[idx];
						} 					
						++i;
					}
				}
            }
			
            if(weightPosUnbound[std::make_pair(pos1, pos2)] > param.weightThreshold) {
				utils::rateArray countvalues = expRatesFalseDetectUnbound.getValues(pos1, pos2);
				// save positions per pair, where nucl1 is mutant and nucl2 is wt
				for(int mnucl=1, i=0; mnucl<5; ++mnucl) {					
					if(wtBase1 != mnucl) {
						int idx = 4*(mnucl-1)+wtBase2-1;
						if(countvalues[idx] > 0) {
							sums_perBaseUnbound[i].insert(countvalues[idx]);
                             totalSum_perBase[i] += countvalues[idx];
						} 
						++i;
					}
				}
            }
			
            // samplewise error estimation
			for(auto boundIt = weightsPerSampleBound.begin(), unboundIt=weightsPerSampleUnbound.begin();  boundIt != weightsPerSampleBound.end(); ++boundIt, ++unboundIt) {
				int boundBarcode = boundIt->first;
				int unboundBarcode = unboundIt->first;
                if(sumsPerSample_perBaseBound.find(boundBarcode) == sumsPerSample_perBaseBound.end()) {
                    (sumsPerSample_perBaseBound[boundBarcode]).resize(3);
                }

                if(sumsPerSample_perBaseUnbound.find(unboundBarcode) == sumsPerSample_perBaseUnbound.end()) {
                    (sumsPerSample_perBaseUnbound[unboundBarcode]).resize(3);
                }
                if(weightsPerSampleBound[boundBarcode][std::make_pair(pos1, pos2)] > param.weightThreshold) {

					utils::rateArray countvalues = expFalseDetectPerSampleBound[boundBarcode].getValues(pos1, pos2);
					//total sum of mutations
					double sum = 0;
					// save positions per pair, where nucl1 is mutant and nucl2 is wt
					for(int mnucl=1, i=0; mnucl<5; ++mnucl) {
						if(wtBase1 != mnucl) {
							int idx = 4*(mnucl-1)+wtBase2-1;
                            if(countvalues[idx] > 0) {                                
                                sumsPerSample_perBaseBound[boundBarcode][i].insert(countvalues[idx]);
                            }
							sum += countvalues[idx];
							++i;
						}
					}

					if(sum > 0) {
						sumsPerSampleBound[boundBarcode].insert(sum);
					}
				}
                if(weightsPerSampleUnbound[unboundBarcode][std::make_pair(pos1, pos2)] > param.weightThreshold) {
					sumsPerSample_perBaseUnbound[unboundBarcode].resize(3);
					utils::rateArray countvalues = expFalseDetectPerSampleUnbound[unboundBarcode].getValues(pos1, pos2);
					//total sum of mutations
					double sum = 0;
					// save positions per pair, where nucl1 is mutant and nucl2 is wt
					for(int mnucl=1, i=0; mnucl<5; ++mnucl) {
						if(wtBase1 != mnucl) {
							int idx = 4*(mnucl-1)+wtBase2-1;
                            if(countvalues[idx] > 0) {
                                sumsPerSample_perBaseUnbound[unboundBarcode][i].insert(countvalues[idx]);
                            }
							sum += countvalues[idx];
							++i;
						}
					}
					if(sum > 0) {
						sumsPerSampleUnbound[unboundBarcode].insert(sum);
					}
				}
			}

		}
		
		//add last item
        if(!(totalSum_perBase[0] == 0 && totalSum_perBase[1] == 0 && totalSum_perBase[2] == 0)) {
			std::vector<double> mediansBound(3);
			std::vector<double> mediansUnbound(3);
			for(int i = 0; i < 3; ++i) {
                if(!(sums_perBaseBound[i]).empty() && !(sums_perBaseUnbound[i]).empty())
                {
                    mediansBound[i] = utils::getMedian(sums_perBaseBound[i]);
                    mediansUnbound[i] = utils::getMedian(sums_perBaseUnbound[i]);
                }
				
				totalSum_perBase[i] = 0;
				sums_perBaseBound[i].clear();
				sums_perBaseUnbound[i].clear();
			}
			data.medianExpKappaBound_perBase[actualPos1] = mediansBound;
			data.medianExpKappaUnbound_perBase[actualPos1] = mediansUnbound;
			
			// total error rate for each sample
            for(auto boundIt = weightsPerSampleBound.begin(), unboundIt=weightsPerSampleUnbound.begin();  boundIt != weightsPerSampleBound.end(); ++boundIt, ++unboundIt)
            {
				int boundBarcode = boundIt->first;
				int unboundBarcode = unboundIt->first;
                if(!(sumsPerSampleBound[boundBarcode]).empty() && !(sumsPerSampleUnbound[unboundBarcode]).empty())
                {
                    data.medianExpKappaTotal_perSampleBound[boundBarcode][actualPos1] = utils::getMedian(sumsPerSampleBound[boundBarcode]);
                    data.medianExpKappaTotal_perSampleUnbound[unboundBarcode][actualPos1] = utils::getMedian(sumsPerSampleUnbound[unboundBarcode]);
                }

                (data.medianExpKappaTotal_perBase_perSampleBound[boundBarcode][actualPos1]).resize(3);
                (data.medianExpKappaTotal_perBase_perSampleUnbound[boundBarcode][actualPos1]).resize(3);
                std::vector<double> mediansBound(3);
                std::vector<double> mediansUnbound(3);
                for(int i= 0; i < 3; ++i) {
                    if(!(sumsPerSample_perBaseBound[boundBarcode][i]).empty() && !(sumsPerSample_perBaseUnbound[unboundBarcode][i]).empty())
                    {
                        mediansBound[i] = utils::getMedian(sumsPerSample_perBaseBound[boundBarcode][i]);
                        mediansUnbound[i] = utils::getMedian(sumsPerSample_perBaseUnbound[unboundBarcode][i]);
                    }
                    sumsPerSample_perBaseBound[boundBarcode][i].clear();
                    sumsPerSample_perBaseUnbound[unboundBarcode][i].clear();
                }
                data.medianExpKappaTotal_perBase_perSampleBound[boundBarcode][actualPos1] = mediansBound;
                data.medianExpKappaTotal_perBase_perSampleUnbound[unboundBarcode][actualPos1] = mediansUnbound;
			}
		} 
	}

    /************************* Compute KD values *********************************/


//    double computeWTError(const string& expDir, int barcode, std::map<int, utils::rateArray>& counts) {
//        ioTools::readExperimentFileAndSum(barcode, expDir, counts);

//        //Number of falsely inserted "real" wildtype U
//        double numMutU = counts[337][3];
//        double wtError = numMutU/counts[337].sum();
//        return wtError;
//    }

//    int wtErrorCorrection(utils::Values& counts, double wtError, int pos1, int pos2, utils::DataContainer& data) {
//        int wtNucl1 = data.ref[pos1];
//        int wtNucl2 = data.ref[pos2];
//        if(pos1 == 337)
//            wtNucl1 = 4;
//        else if(pos2 == 337)
//            wtNucl2 = 4;

//        const double wt1Counts = counts[(wtNucl1-1)*4+wtNucl2-1];
	
//        double newWtWtVal = wt1Counts *(1 - wtError);
//        int dev = 0;
//        if(newWtWtVal < 0) {
//            dev = -1;
//        } else if(newWtWtVal > 0){
//            dev = 1;
//        }

//        counts[(wtNucl1-1)*4+wtNucl2-1] = newWtWtVal;
//        return dev;

//    }
	
    void computeKDvalues(const string& expDir, utils::Parameter& param, utils::DataContainer& data){
        std::cout << "Compute KD Values...." << std::endl;
		
        //clear already computed results
        data.clearRawKDCriteria();

        utils::sampleContainer::iterator boundIt, unboundIt, wtBoundIt, wtUnboundIt;
		
		//reserve space for upperbound of possible experiments (otherwise it would need O(vecSize) everytime it's reallocating)
		std::size_t maximalVecSize = (param.seqEnd-param.seqBegin+1)*(data.bound.size());
		
// 		data.positions.reserve(param.seqEnd-param.seqBegin+1);

		//1d counts per barcode, per position
//		std::map<int, std::map<int, utils::rateArray>> counts1dBound;
//		std::map<int, std::map<int, utils::rateArray>> counts1dUnbound;
		//compute wt-error of in virion data
//		if(param.virion) {
//			for(boundIt=data.bound.begin(), unboundIt=data.unbound.begin(); boundIt != data.bound.end(); ++boundIt, ++unboundIt) {

//				if((*boundIt).library != 0) {
					
//					double wtError = computeWTError(expDir+"/1d", (*boundIt).barcode, counts1dBound[(*boundIt).barcode]);
//					data.wtErrors[(*boundIt).barcode] = wtError;
//					wtError = computeWTError(expDir+"/1d", (*unboundIt).barcode, counts1dUnbound[(*unboundIt).barcode]);
//					data.wtErrors[(*unboundIt).barcode] = wtError;
//				}

//			}
//		}


        int numberOfExp = 0;
        for(boundIt=data.bound.begin(), unboundIt=data.unbound.begin(); boundIt != data.bound.end(); ++boundIt, ++unboundIt) {
            if((*boundIt).library != 0) {
                ++numberOfExp;
                int boundBarcode = (*boundIt).barcode;
                int unboundBarcode = (*unboundIt).barcode;
                utils::countsPerPosPair boundCountsPP;
                utils::countsPerPosPair unboundCountsPP;
                ioTools::readExperimentFile(boundBarcode, expDir+"/2d", boundCountsPP);
                ioTools::readExperimentFile(unboundBarcode, expDir+"/2d", unboundCountsPP);

                //first count positions, normalize per experiment and add afterwards to vector
                utils::WeightPerPosPair weightPerPosPairSingleExpBound;
                utils::WeightPerPosPair weightPerPosPairSingleExpUnbound;

                //only consider data in the given interval
                //#pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, expDir, param, data, maximalVecSize) private(boundIt, unboundIt)
                for(int pos1 = param.seqBegin; pos1 <= param.seqEnd; ++pos1) {
                    if(boundCountsPP.find(pos1) != boundCountsPP.end() && unboundCountsPP.find(pos1) != unboundCountsPP.end()) {
                    if(numberOfExp == 1) {
                        data.signal2noiseBound_perPos[pos1].resize(3);
                        data.signal2noiseUnbound_perPos[pos1].resize(3);
                        data.totalRawRelKD_perPos[pos1].resize(3);
                        for(int i=0; i<3; ++i) {
                            data.signal2noiseBound_perPos[pos1][i].reserve(maximalVecSize);
                            data.signal2noiseUnbound_perPos[pos1][i].reserve(maximalVecSize);
                            data.totalRawRelKD_perPos[pos1][i].reserve(maximalVecSize);
                        }
                    }

                    for(auto pos2It = boundCountsPP[pos1].begin(); pos2It != boundCountsPP[pos1].end(); ++pos2It)
                    {
                        int pos2 = pos2It->first;


                        if(pos2 >= param.seqBegin && pos2 <= param.seqEnd) {

                            utils::Values boundCounts(boundCountsPP[pos1][pos2]);
                            utils::Values unboundCounts(unboundCountsPP[pos1][pos2]);
							
							int wtBase1 = data.ref[pos1];
							int wtBase2 = data.ref[pos2];

                            //für später
//							if(param.virion) {
	
//								wtErrorCorrection(boundCounts, data.wtErrors[(*boundIt).barcode], pos1, pos2, data, counts1dBound[(*boundIt).barcode][pos1]);

//								int dev = wtErrorCorrection(unboundCounts, data.wtErrors[(*unboundIt).barcode], pos1, pos2, data, counts1dUnbound[(*unboundIt).barcode][pos1]);
// 								if(dev > 0)
// 									++moreZero;
// 								else if(dev <0)
// 									++lessZero;
// 								else
// 									std::cout << "dev " << dev << std::endl;
								
// 								if(pos1 == 337)
// 									wtBase1 = 1;
// 								if(pos2 == 337)
// 									wtBase2 = 1;

//							}
							
							//counts of Wt - Wt
							int boundNumWtWt = boundCounts[(wtBase1-1)*4+wtBase2-1];
							int unboundNumWtWt = unboundCounts[(wtBase1-1)*4+wtBase2-1];
							
							//counts of all sequences of this position pair
							double boundCountSum = boundCounts.sum();
							double unboundCountSum = unboundCounts.sum();					

							weightPerPosPairSingleExpBound.add(pos1, pos2, boundCountSum);
							weightPerPosPairSingleExpUnbound.add(pos1,pos2,unboundCountSum);

							
							for(int mnucl=1, i=0; mnucl<5 && i<3; ++mnucl) {
								if(wtBase1 != mnucl) {
									int idx = 4*(mnucl-1)+wtBase2-1;
									//****for each base *****

                                    //if errors are considered indipendently for each sample: find respective barcode of wildtype
                                    int wtBoundBarcode = boundBarcode;
                                    int wtUnboundBarcode = unboundBarcode;
                                    if(!param.joinErrors) {
                                        for(wtBoundIt=data.bound.begin(), wtUnboundIt=data.unbound.begin(); wtBoundIt != data.bound.end(); ++wtBoundIt, ++wtUnboundIt)
                                        {
                                            if((*wtBoundIt).library == 0 && (*wtBoundIt).name == (*boundIt).name)
                                                wtBoundBarcode = (*wtBoundIt).barcode;
                                            if((*wtUnboundIt).library == 0 && (*wtUnboundIt).name == (*unboundIt).name)
                                                wtUnboundBarcode = (*wtUnboundIt).barcode;
                                        }
                                    }

                                    if((param.joinErrors && (data.medianExpKappaBound_perBase.find(pos1) != data.medianExpKappaBound_perBase.end() && data.medianExpKappaBound_perBase[pos1][i]
                                            && data.medianExpKappaUnbound_perBase.find(pos1) != data.medianExpKappaUnbound_perBase.end() && data.medianExpKappaUnbound_perBase[pos1][i]))
                                            || (!param.joinErrors && data.medianExpKappaTotal_perBase_perSampleBound[wtBoundBarcode].find(pos1) != data.medianExpKappaTotal_perBase_perSampleBound[wtBoundBarcode].end()
                                                && data.medianExpKappaTotal_perBase_perSampleBound[wtBoundBarcode][pos1][i]
                                                && data.medianExpKappaTotal_perBase_perSampleUnbound[wtUnboundBarcode].find(pos1) != data.medianExpKappaTotal_perBase_perSampleUnbound[wtUnboundBarcode].end()
                                                && data.medianExpKappaTotal_perBase_perSampleUnbound[wtUnboundBarcode][pos1][i])) {
										

                                        double noiseBound;
                                        double noiseUnbound;

                                        if(param.joinErrors)
                                        {
                                            noiseBound = data.medianExpKappaBound_perBase[pos1][i];
                                            noiseUnbound = data.medianExpKappaUnbound_perBase[pos1][i];
                                        } else
                                        {
                                            noiseBound = data.medianExpKappaTotal_perBase_perSampleBound[wtBoundBarcode][pos1][i];
                                            noiseUnbound = data.medianExpKappaTotal_perBase_perSampleUnbound[wtUnboundBarcode][pos1][i];
                                        }

										double denominator = boundCounts[idx]/boundNumWtWt-noiseBound;
										double nominator = unboundCounts[idx]/unboundNumWtWt-noiseUnbound; 

										data.signal2noiseBound_perPos[pos1][i].push_back(boundCounts[idx]/(boundNumWtWt*noiseBound));
										data.signal2noiseUnbound_perPos[pos1][i].push_back(unboundCounts[idx]/(unboundNumWtWt*noiseUnbound));

										if(std::isinf(unboundCounts[idx]/(unboundNumWtWt*noiseUnbound)))
											//not enough signal at supernatent => signal-to-noise-ratio too bad
                                            data.totalRawRelKD_perPos[pos1][i].push_back(std::numeric_limits<double>::quiet_NaN());
										else if(denominator <= 0)
											//not enough signal for bound
                                            data.totalRawRelKD_perPos[pos1][i].push_back(-1);
										else if(nominator <= 0)
											//not enough signal for unbound
                                            data.totalRawRelKD_perPos[pos1][i].push_back(-10);
										else
                                            data.totalRawRelKD_perPos[pos1][i].push_back(nominator/denominator);
									}
									
									++i;
								}	
							}

							
							data.numSeqPerPosPairBound[pos1].push_back(boundCountSum);
							data.numSeqPerPosPairUnbound[pos1].push_back(unboundCountSum);
							
                        } //end of if pos2 in interval

                    } // end of pos2 loop



                    } // end of if within sequence interval statement
					
                } // end of pos1 loop

                //normalize per experiment
                weightPerPosPairSingleExpBound.normalize();
                weightPerPosPairSingleExpUnbound.normalize();
                //weightPerPosPairSingleExpTotal.normalize();

                //...and then add to list
                //TODO: make more efficient
                int pos2 = 1;
                for(int pos1 = param.seqBegin; pos1 <= param.seqEnd; ++pos1) {                    
                    if(data.medianExpKappaBound_perBase.find(pos1) != data.medianExpKappaBound_perBase.end() && data.medianExpKappaUnbound_perBase.find(pos1) != data.medianExpKappaUnbound_perBase.end())
                    {
                        for(utils::countsPerSecondPos::iterator boundPos2It = boundCountsPP[pos1].begin(); boundPos2It != boundCountsPP[pos1].end(); ++boundPos2It) {
                            pos2 = boundPos2It->first;
                            if(pos2 >= param.seqBegin && pos2 <= param.seqEnd) {
                                data.positionWeightsBound[pos1].push_back(weightPerPosPairSingleExpBound.getValue(pos1, pos2));
                                data.positionWeightsUnbound[pos1].push_back(weightPerPosPairSingleExpUnbound.getValue(pos1, pos2));
                            }
                        }
                    }
                }

            } // end of if not wt library statement			
        } // end of experiment loop
	}
	
    void applyQualityCriteria(utils::Parameter& param, utils::DataContainer& data) {
		
        std::cout << "Apply Quality Criteria... " << std::endl;
		//post-processing based on quality criteria

        //delete old values before computing
        data.clearKDQualityCriteria();
        data.totalRelKD_perPos = data.totalRawRelKD_perPos;

		//count number of samples which are valid (not nan)
// 		std::map<int, std::vector<int>> numberOfKDs;
		std::map<int, std::vector<int>> numberOfKDs_smallerZero;
		std::map<int, std::vector<int>> numberOfKDs_greaterZero;

		
		std::vector<double> pvalues;
		std::map<int, std::vector<double>> pvaluesIdx;
        int numPValues = 0;
        for(int pos1 = param.seqBegin; pos1 <= param.seqEnd; ++pos1) {
            if(data.medianExpKappaBound_perBase.find(pos1) != data.medianExpKappaBound_perBase.end()
                    && data.medianExpKappaUnbound_perBase.find(pos1) != data.medianExpKappaUnbound_perBase.end()
                    && data.totalRelKD_perPos.find(pos1) != data.totalRelKD_perPos.end())
            {
                int wtBase1 = data.ref[pos1];

                data.lowerLimitsKD_perPos[pos1].resize(3);
                data.upperLimitsKD_perPos[pos1].resize(3);
                pvaluesIdx[pos1].resize(3);
                data.numberOfKDs[pos1].resize(3);
                numberOfKDs_smallerZero[pos1].resize(3);
                numberOfKDs_greaterZero[pos1].resize(3);
                data.pvalues[pos1].resize(3);
                data.KDmedians[pos1].resize(3);


                std::vector<std::multiset<double>> validKdsForPercentile(3);

                //remember indices where upper or lower estimate is made and has to be replaced by median/percentile
                std::vector<std::vector<int>> upperBoundIdx(3);
                upperBoundIdx[0].reserve(data.numSeqPerPosPairBound[pos1].size());
                upperBoundIdx[1].reserve(data.numSeqPerPosPairBound[pos1].size());
                upperBoundIdx[2].reserve(data.numSeqPerPosPairBound[pos1].size());
                std::vector<std::vector<int>> lowerBoundIdx(3);
                lowerBoundIdx[0].reserve(data.numSeqPerPosPairBound[pos1].size());
                lowerBoundIdx[1].reserve(data.numSeqPerPosPairBound[pos1].size());
                lowerBoundIdx[2].reserve(data.numSeqPerPosPairBound[pos1].size());

    //            #pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, param, data, pos1, wtBase1, validKdsForPercentile, numberOfKDs_smallerZero, numberOfKDs_greaterZero, upperBoundIdx, lowerBoundIdx)
                for(unsigned int i=0; i < data.numSeqPerPosPairBound[pos1].size(); ++i) {
                    for(int mnucl=1, mut=0; mnucl<5 && mut<3; ++mnucl) {

                        if(wtBase1 != mnucl) {

                            if(data.totalRelKD_perPos[pos1][mut].size() > 0)
                            {
                                  if((data.signal2noiseBound_perPos[pos1][mut][i] < param.minSignal2NoiseStrength
                                    && data.signal2noiseUnbound_perPos[pos1][mut][i] < param.minSignal2NoiseStrength)
                                    || data.positionWeightsBound[pos1][i] < param.weightThreshold || data.numSeqPerPosPairBound[pos1][i] < param.minimumNrCalls
                                    || data.positionWeightsUnbound[pos1][i] < param.weightThreshold || data.numSeqPerPosPairUnbound[pos1][i] < param.minimumNrCalls) {

                                    data.totalRelKD_perPos[pos1][mut][i] = std::numeric_limits<double>::quiet_NaN();

                                } else if(data.signal2noiseBound_perPos[pos1][mut][i] < param.minSignal2NoiseStrength
                                    && data.signal2noiseUnbound_perPos[pos1][mut][i] >= param.minSignal2NoiseStrength) {
                                    ++data.lowerLimitsKD_perPos[pos1][mut];
                                    lowerBoundIdx[mut].push_back(i);

                                } else if(data.signal2noiseUnbound_perPos[pos1][mut][i] < param.minSignal2NoiseStrength
                                    && data.signal2noiseBound_perPos[pos1][mut][i] >= param.minSignal2NoiseStrength) {
                                    ++data.upperLimitsKD_perPos[pos1][mut];
                                    upperBoundIdx[mut].push_back(i);
                                } else {
                                    validKdsForPercentile[mut].insert(data.totalRelKD_perPos[pos1][mut][i]);
                                    //count number of ressamplings (whicht are not nan) and numbers of binding increasing and decreasing mutations
                                    double KDvalue_log2 = log2(data.totalRelKD_perPos[pos1][mut][i]);
                                    //careful: not all compiler like isnan
                                    if(!std::isnan(KDvalue_log2)) {
                                        ++data.numberOfKDs[pos1][mut];

                                        if(KDvalue_log2 >= 0) {
                                            ++numberOfKDs_greaterZero[pos1][mut];
                                        }

                                        if(KDvalue_log2 <= 0) {
                                            ++numberOfKDs_smallerZero[pos1][mut];
                                        }
                                    }
                                }
                            }


                            ++mut;
                        }
                    }
                }

                //compute raw pvalues for those where number of counted KDs is high enough
                for(int mnucl=1, mut=0; mnucl<5 && mut<3; ++mnucl) {
                    if(wtBase1 != mnucl) {
                            double median;
                            double perc95;
                            double perc5;

                            if(data.numberOfKDs[pos1][mut] > 0 && !(validKdsForPercentile[mut]).empty()) {
                                median = utils::getPercentile(validKdsForPercentile[mut], 50);
                                perc95 = utils::getPercentile(validKdsForPercentile[mut], 95);
                                perc5 = utils::getPercentile(validKdsForPercentile[mut], 5);



        //                        #pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, param, data, mut, pos1, median, perc95, perc5, numberOfKDs_smallerZero, numberOfKDs_greaterZero, lowerBoundIdx)
                                for(unsigned int i=0; i<lowerBoundIdx[mut].size(); ++i) {
                                    int idx = lowerBoundIdx[mut][i];

                                    if(data.numberOfKDs[pos1][mut] == 0)
                                        data.totalRelKD_perPos[pos1][mut][idx] = std::numeric_limits<double>::quiet_NaN();
                                    else if(param.putMedian)
                                        data.totalRelKD_perPos[pos1][mut][idx] = median;
                                    else
                                        data.totalRelKD_perPos[pos1][mut][idx] = perc95;
                                    //count number of resamplings (which are not nan) and numbers of binding increasing and decreasing mutations
                                    double KDvalue_log2 = log2(data.totalRelKD_perPos[pos1][mut][idx]);
                                    //careful: not all compiler like isnan
                                    if(!std::isnan(KDvalue_log2)) {
                                        ++data.numberOfKDs[pos1][mut];
                                        if(KDvalue_log2 >= 0) {
                                            ++numberOfKDs_greaterZero[pos1][mut];
                                        } else if(KDvalue_log2 <= 0) {
                                            ++numberOfKDs_smallerZero[pos1][mut];
                                        }
                                    }

                                }


        //                        #pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, param, data, mut, pos1, median, perc95, perc5, numberOfKDs_smallerZero, numberOfKDs_greaterZero, upperBoundIdx)
                                for(unsigned int i=0; i<upperBoundIdx[mut].size(); ++i) {
                                    int idx = upperBoundIdx[mut][i];

                                    if(data.numberOfKDs[pos1][mut] == 0)
                                        data.totalRelKD_perPos[pos1][mut][idx] = std::numeric_limits<double>::quiet_NaN();
                                    else if(param.putMedian)
                                        data.totalRelKD_perPos[pos1][mut][idx] = median;
                                    else
                                        data.totalRelKD_perPos[pos1][mut][idx] = perc5;

                                    //count number of ressamplings (which are not nan) and numbers of binding increasing and decreasing mutations
                                    double KDvalue_log2 = log2(data.totalRelKD_perPos[pos1][mut][idx]);
                                    //careful: not all compiler like isnan
                                    if(!std::isnan(KDvalue_log2)) {
                                        ++data.numberOfKDs[pos1][mut];

                                        if(KDvalue_log2 >= 0) {
                                            ++numberOfKDs_greaterZero[pos1][mut];
                                        } else if(KDvalue_log2 <= 0) {
                                            ++numberOfKDs_smallerZero[pos1][mut];
                                        }
                                    }

                                }
                            }


                            if(data.numberOfKDs[pos1][mut] >= param.minNumberEstimatableKDs) {
                                //collect all  pvalues for Benjamini Hochberg method
                                pvalues.push_back(min(numberOfKDs_smallerZero[pos1][mut], numberOfKDs_greaterZero[pos1][mut])/(double)data.numberOfKDs[pos1][mut]);
                                //rememeber index of pvalue
                                pvaluesIdx[pos1][mut] = numPValues;
                                //count number of considered pvalues
                                ++numPValues;
                            } else {
                                pvaluesIdx[pos1][mut] = std::numeric_limits<double>::quiet_NaN();
                            }

                        ++mut;
                    }
                }
            }

		}
		
		//P-value correction for multiple testing (Benjamini Hochberg method)
		std::vector<size_t> sortedPValueIndices = utils::sort_indexes(pvalues);

		std::vector<bool> pvalueSmallerAlpha(pvalues.size(), false);
		
//		#pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, param, data, numPValues, pvalues, sortedPValueIndices, pvalueSmallerAlpha)
		for(int i = 0; i < numPValues; ++i) {
				pvalues[sortedPValueIndices[i]] *=((double)numPValues/(i+1.f));
				//remember pvalues smaller alpha
				pvalueSmallerAlpha[sortedPValueIndices[i]] = pvalues[sortedPValueIndices[i]] < param.alpha;	
		}

// 		for(int pos1 = param.seqBegin; pos1 <= param.seqEnd; ++pos1) 
		data.positions.reserve(pvaluesIdx.size());
		for(auto it = pvaluesIdx.begin(); it != pvaluesIdx.end(); ++it) {
			int pos1 = it->first;
			
			
			int wtBase1 = data.ref[pos1];
			
			bool maxAlpha = false;
			size_t maxMut = 0;
			double maxMedian = 0;

			for(int mnucl=1, mut=0; mnucl<5 && mut<3; ++mnucl) {
				if(wtBase1 != mnucl) {
					if(!std::isnan(pvaluesIdx[pos1][mut])) {
						
						data.pvalues[pos1][mut] = pvalues[pvaluesIdx[pos1][mut]];
						
// 						find maximum median, if medians < alpha are existant, consider only those. if not consider all
                        if(!(data.totalRelKD_perPos[pos1][mut]).empty())
                            data.KDmedians[pos1][mut] = utils::getPercentile(data.totalRelKD_perPos[pos1][mut], 50);

						double currentMedian = abs(log2(data.KDmedians[pos1][mut]));
						bool isAlpha = pvalueSmallerAlpha[pvaluesIdx[pos1][mut]];
                        //ignore in virion cases where wt=A and mut=G
						if((!param.virion || !(wtBase1 == 1 && mnucl == 3)) && ((maxMedian < currentMedian && maxAlpha == isAlpha) || (!maxAlpha && isAlpha))) {
//						if((maxMedian < currentMedian && maxAlpha == isAlpha) || (!maxAlpha && isAlpha)) {
							maxAlpha = isAlpha;
							maxMut = mnucl;
							maxMedian = currentMedian;
						}
					} else {
						data.pvalues[pos1][mut] = std::numeric_limits<double>::quiet_NaN();
						data.KDmedians[pos1][mut] = std::numeric_limits<double>::quiet_NaN();
					}
					++mut;
				}
			}			
			
			if(maxMut > 0) {
				
				data.maxMut[pos1] = maxMut;
				//consider only valid positions
				data.positions.push_back(pos1);				
			}	
		}	
	}
}
