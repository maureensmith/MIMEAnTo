#include "processing.hpp"

#include <iostream>
#include <regex>
#include <iterator>
#include <sstream>
#include <valarray>

//namespace fs = boost::filesystem;
namespace processing {


/****************** Error Estimation **************************************************/

    void countMutationFrequency(int barcode, int pos1, int pos2, int nucl1, int nucl2, utils::rateArray& countsPP,
                                utils::WeightPerPosPair& weightPos, std::map<int, utils::WeightPerPosPair>& weightsPerSample,
                                utils::RatesPerPosPair& expRatesFalseDetect,  std::map<int, utils::RatesPerPosPair>& expFalseDetectPerSample) {
        utils::Values counts(countsPP);

        int numWtWt = counts[(nucl1-1)*4+nucl2-1];
        int seqSum = counts.sum();

        //count all sequences for a pair combination
        weightPos.add(pos1, pos2, seqSum);
        counts.divide(numWtWt);
        //TODO refactor: adding?? wieso nehme ich hier den mean von allen samples? joining error sollte sein: median aus allen resamplings
        expRatesFalseDetect.add(pos1, pos2, counts);
        //TODO hier wird nicht geaddet wil getUniqueWildTypeBarcodes aufgerufen wurden.
        // also wird jeder barcode pos1 pos2 nur einmal aufgerufen. SCHÖNER machen!
        expFalseDetectPerSample[barcode].add(pos1, pos2, counts);
        weightsPerSample[barcode].add(pos1, pos2, seqSum);
    }

    //TODO refactor: Single??? Nee. Außerdem aufteilen in eine funktion pro Methode
    void getSingleErrorsAndWeights(const string& expDir, utils::Parameter& param, utils::refMap& ref,
                                  utils::WeightPerPosPair& weightPos, std::map<int, utils::WeightPerPosPair>& weightsPerSample,
                                  utils::RatesPerPosPair& expRatesFalseDetect, std::map<int, utils::RatesPerPosPair>& expFalseDetectPerSample,
                                   std::vector<int>& barcodes)
    {
        //utils::sampleContainer::iterator sampleIt;
        int numberOfExp = 0;

        for(auto barcode : barcodes) {
       // TODO weg, wurde umgebaut //for each experiment pair with wildtype(library=0)
       // for(sampleIt=samples.begin(); sampleIt != samples.end(); ++sampleIt) {
       //     //for errors only exermine wt samples (lib = 0)
       //     if((*sampleIt).library == 0) {
                ++numberOfExp;
                utils::countsPerPosPair countsPP;
                ioTools::readExperimentFile(barcode, expDir+"/2d", countsPP);

                for(int pos1 = param.seqBegin; pos1 <= param.seqEnd; ++pos1) {
                    int pos2 = 1;
                    for(auto pos2It = countsPP[pos1].begin(); pos2It != countsPP[pos1].end() && pos2 <= param.seqEnd; ++pos2It) {
                        pos2 = pos2It->first;
                        //only consider data in the given interval
                        if(pos1 >= param.seqBegin && pos1 <= param.seqEnd && pos2 >= param.seqBegin && pos2 <= param.seqEnd) {
                            int nucl1 = ref[pos1];
                            int nucl2 = ref[pos2];
                            countMutationFrequency(barcode, pos1, pos2, nucl1, nucl2, pos2It->second, weightPos, weightsPerSample,
                                                   expRatesFalseDetect, expFalseDetectPerSample);
                        }
                    }
                }
                weightsPerSample[barcode].normalize();
       //         barcodes.push_back(barcode);
       //     }
        }

        //TODO refactor weg???
        // average Rate of all experiments
        expRatesFalseDetect.divide(numberOfExp);

        //normalize by maximamum value for position 1
        weightPos.normalize();

    }


    void estimateErrorValuesForActualPosition(std::vector<int>& barcodes, int actualPos1,
                                             std::map<int, std::multiset<double>>& sumsPerSample, std::vector<std::multiset<double>>& sums_perBase,
                                             std::map<int, std::vector<std::multiset<double>>>& sumsPerSample_perBase,
                                             std::map<int, std::vector<double>>& medianExpKappa_perBase,
                                             std::map<int, std::map<int, double>>& medianExpKappaTotal_perSample,
                                             std::map<int, std::map<int, double>>& perc75ExpKappaTotal_perSample,
                                             std::map<int, std::map<int, double>>& perc25ExpKappaTotal_perSample,
                                             std::map<int, std::map<int, std::vector<double>>>& medianExpKappaTotal_perBase_perSample)
    {
        //save collected values for last position, refresh the containers for next position
        std::vector<double> medians(3);
        for(int i = 0; i < 3; ++i) {
            if(sums_perBase[i].size() > 0)
                medians[i] = utils::getMedian(sums_perBase[i]);

            sums_perBase[i].clear();
        }
        medianExpKappa_perBase[actualPos1] = medians;
        // total error rate for each sample
        for(auto barcodeIt = barcodes.begin(); barcodeIt != barcodes.end(); ++barcodeIt) {

            int barcode = *barcodeIt;
            if(!(sumsPerSample[barcode]).empty())
            {
                medianExpKappaTotal_perSample[barcode][actualPos1] = utils::getMedian(sumsPerSample[barcode]);
                perc25ExpKappaTotal_perSample[barcode][actualPos1] = utils::getPercentile(sumsPerSample[barcode], 25);
                perc75ExpKappaTotal_perSample[barcode][actualPos1] = utils::getPercentile(sumsPerSample[barcode], 75);
            }

            //TODO und das hat ganz gefehlt
            sumsPerSample[barcode].clear();

            //in case the same sample is used for different settings
            if(medianExpKappaTotal_perBase_perSample[barcode][actualPos1].empty()) {
                (medianExpKappaTotal_perBase_perSample[barcode][actualPos1]).resize(3);

                std::vector<double> medians(3);
                for(int i= 0; i < 3; ++i)
                {
                    if(!(sumsPerSample_perBase[barcode][i]).empty())
                    {   
                        medians[i] = utils::getMedian(sumsPerSample_perBase[barcode][i]);
                    } else
                    {
                        medians[i] = 0;
                    }
                    //TODO auskommentiert?
                    sumsPerSample_perBase[barcode][i].clear();
                }

                medianExpKappaTotal_perBase_perSample[barcode][actualPos1] = medians;

            }
        }
    }

    void collectValuesForActualPosition(int wtBase1, int wtBase2, int pos1, int pos2, double weightThreshold, std::vector<int>& barcodes,
                                        std::vector<double>& totalSum_perBase, std::vector<std::multiset<double>>& sums_perBase,
                                        std::map<int, std::multiset<double>>& sumsPerSample, std::map<int, std::vector<std::multiset<double>>>& sumsPerSample_perBase,
                                        utils::RatesPerPosPair& expRatesFalseDetect, std::map<int, utils::RatesPerPosPair>& expFalseDetectPerSample,
                                        utils::WeightPerPosPair& weightPos, std::map<int, utils::WeightPerPosPair>& weightsPerSample)
    {
        if(weightPos[std::make_pair(pos1, pos2)] > weightThreshold) {
            utils::rateArray countvalues = expRatesFalseDetect.getValues(pos1, pos2);
            // save positions per pair, where nucl1 is mutant and nucl2 is wt
            for(int mnucl=1, i=0; mnucl<5; ++mnucl) {
                if(wtBase1 != mnucl) {
                    int idx = 4*(mnucl-1)+wtBase2-1;
                    if(countvalues[idx] > 0) {
                        //TODO war vorher ja der mean aller samples für join (glaub ich), also besser unten hin tun für über alle samples gelaufen wird
                        //nee...was soll das sein?
                        sums_perBase[i].insert(countvalues[idx]);
                         totalSum_perBase[i] += countvalues[idx];
                    }
                    ++i;
                }
            }
        }

        // samplewise error estimation
        for(auto barcodeIt = barcodes.begin(); barcodeIt != barcodes.end(); ++barcodeIt) {
            int barcode = *barcodeIt;
//            if(sumsPerSample_perBase.find(barcode) == sumsPerSample_perBase.end())
//                (sumsPerSample_perBase[barcode]).resize(3);

            if(weightsPerSample[barcode][std::make_pair(pos1, pos2)] > weightThreshold) {
                if(sumsPerSample_perBase.find(barcode) == sumsPerSample_perBase.end())
                    (sumsPerSample_perBase[barcode]).resize(3);

                utils::rateArray countvalues = expFalseDetectPerSample[barcode].getValues(pos1, pos2);
                //total sum of mutations
                double sum = 0;
                // save positions per pair, where nucl1 is mutant and nucl2 is wt
                for(int mnucl=1, i=0; mnucl<5; ++mnucl) {
                    if(wtBase1 != mnucl) {
                        int idx = 4*(mnucl-1)+wtBase2-1;
                        if(countvalues[idx] > 0 && !std::isnan(countvalues[idx]))
                            sumsPerSample_perBase[barcode][i].insert(countvalues[idx]);
                        sum += countvalues[idx];
                        //std::cout << pos1 <<" "<<pos2 << " " << barcode << " Multiset " << i << std::endl;
                        ++i;

                    }

                }
                if(sum > 0)
                    sumsPerSample[barcode].insert(sum);
            }
        }
    }

    std::vector<int> getUniqueWildTypeBarcodes(utils::sampleContainer& samples) {
       std::vector<int> wtBarcodes;
       //only uniquebarcodes: in case a barcode for the wildtype is used for different mut barcodes,
       //it appears more often in the vector (to be able to iterate over mut and wt simultaneously)
       for(auto sampleIt=samples.begin(); sampleIt != samples.end(); ++sampleIt) {
           //for errors only exermine wt samples (lib = 0)
           if((*sampleIt).library == 0) {
               wtBarcodes.push_back((*sampleIt).barcode);
           }
       }

       std::sort(wtBarcodes.begin(), wtBarcodes.end());
       wtBarcodes.erase( unique( wtBarcodes.begin(), wtBarcodes.end() ), wtBarcodes.end() );
       return wtBarcodes;
   }

    void estimateError(const string& expDir, utils::Parameter& param, utils::DataContainer& data) {
		std::cout << "Estimate Error...." << std::endl;

        //clear data before computing
        data.clearErrors();

        //Count number of wt samples for each pair of positions
        utils::WeightPerPosPair weightPosBound;
        utils::WeightPerPosPair weightPosUnbound;
        //expected rate of false detection (error rate)
        utils::RatesPerPosPair expRatesFalseDetectBound;
        utils::RatesPerPosPair expRatesFalseDetectUnbound;

        //for the comparison of each sample (error plot and variation coefficient)
        std::map<int, utils::WeightPerPosPair> weightsPerSampleBound;
        std::map<int, utils::WeightPerPosPair> weightsPerSampleUnbound;

        std::map<int, utils::RatesPerPosPair> expFalseDetectPerSampleBound;
        std::map<int, utils::RatesPerPosPair> expFalseDetectPerSampleUnbound;

        //barcodes wor the wildtyp samples for easier iteration
        std::vector<int> boundBarcodes = getUniqueWildTypeBarcodes(data.bound);
        std::vector<int> unboundBarcodes = getUniqueWildTypeBarcodes(data.unbound);

        getSingleErrorsAndWeights(expDir, param, data.ref, weightPosBound, weightsPerSampleBound, expRatesFalseDetectBound, expFalseDetectPerSampleBound, boundBarcodes);
        getSingleErrorsAndWeights(expDir, param, data.ref, weightPosUnbound, weightsPerSampleUnbound, expRatesFalseDetectUnbound, expFalseDetectPerSampleUnbound, unboundBarcodes);

//        //only uniquebarcodes: in case a barcode for the wildtype is used for different mut barcodes,
//        //it appears more often in the vector (to be able to iterate over mut and wt simultaneously)
//        std::sort(boundBarcodes.begin(), boundBarcodes.end());
//        boundBarcodes.erase( unique( boundBarcodes.begin(), boundBarcodes.end() ), boundBarcodes.end() );

//        std::sort(unboundBarcodes.begin(), unboundBarcodes.end());
//        unboundBarcodes.erase( unique( unboundBarcodes.begin(), unboundBarcodes.end() ), unboundBarcodes.end() );




		int actualPos1 = param.seqBegin;
		std::map<int, std::multiset<double>> sumsPerSampleBound;
		std::map<int, std::multiset<double>> sumsPerSampleUnbound;

		std::vector<std::multiset<double>> sums_perBaseBound(3);
		std::vector<std::multiset<double>> sums_perBaseUnbound(3);
        //for join errors
		std::map<int, std::vector<std::multiset<double>>> sumsPerSample_perBaseBound;
		std::map<int, std::vector<std::multiset<double>>> sumsPerSample_perBaseUnbound;

		std::vector<double> totalSum_perBase(3);

        //for loop with bound (but unbound should have the same amount of position pairs)
        for(utils::RatesPerPosPair::iterator it = expRatesFalseDetectBound.begin(); it != expRatesFalseDetectBound.end(); ++it) {
            int pos1 = expRatesFalseDetectBound.getFirstPos(it);
            int pos2 = expRatesFalseDetectBound.getSecondPos(it);
			int wtBase1 = data.ref[pos1];
			int wtBase2 = data.ref[pos2];

			if(pos1 != actualPos1) {
                if(!(totalSum_perBase[0] == 0 && totalSum_perBase[1] == 0 && totalSum_perBase[2] == 0)) {
                    estimateErrorValuesForActualPosition(boundBarcodes, actualPos1, sumsPerSampleBound, sums_perBaseBound, sumsPerSample_perBaseBound,
                                                             data.medianExpKappaBound_perBase, data.medianExpKappaTotal_perSampleBound,
                                                             data.perc75ExpKappaTotal_perSampleBound, data.perc25ExpKappaTotal_perSampleBound,
                                                             data.medianExpKappaTotal_perBase_perSampleBound);
                    estimateErrorValuesForActualPosition(unboundBarcodes, actualPos1, sumsPerSampleUnbound, sums_perBaseUnbound, sumsPerSample_perBaseUnbound,
                                                             data.medianExpKappaUnbound_perBase, data.medianExpKappaTotal_perSampleUnbound,
                                                             data.perc75ExpKappaTotal_perSampleUnbound, data.perc25ExpKappaTotal_perSampleUnbound,
                                                             data.medianExpKappaTotal_perBase_perSampleUnbound);
                }

                //clear here because it's used for both bound and unbound
                for(int i = 0; i < 3; ++i)
                    totalSum_perBase[i] = 0;
				actualPos1 = pos1;
			}
			
            collectValuesForActualPosition(wtBase1, wtBase2, pos1, pos2, param.weightThreshold, boundBarcodes, totalSum_perBase, sums_perBaseBound,
                                           sumsPerSampleBound, sumsPerSample_perBaseBound, expRatesFalseDetectBound, expFalseDetectPerSampleBound,
                                           weightPosBound, weightsPerSampleBound);
            collectValuesForActualPosition(wtBase1, wtBase2, pos1, pos2, param.weightThreshold, unboundBarcodes, totalSum_perBase, sums_perBaseUnbound,
                                           sumsPerSampleUnbound, sumsPerSample_perBaseUnbound, expRatesFalseDetectUnbound, expFalseDetectPerSampleUnbound,
                                           weightPosUnbound, weightsPerSampleUnbound);
		}
		
		//add last item
        estimateErrorValuesForActualPosition(boundBarcodes, actualPos1, sumsPerSampleBound, sums_perBaseBound, sumsPerSample_perBaseBound,
                                                 data.medianExpKappaBound_perBase, data.medianExpKappaTotal_perSampleBound,
                                                 data.perc75ExpKappaTotal_perSampleBound, data.perc25ExpKappaTotal_perSampleBound,
                                                 data.medianExpKappaTotal_perBase_perSampleBound);
        estimateErrorValuesForActualPosition(unboundBarcodes, actualPos1, sumsPerSampleUnbound, sums_perBaseUnbound, sumsPerSample_perBaseUnbound,
                                                 data.medianExpKappaUnbound_perBase, data.medianExpKappaTotal_perSampleUnbound,
                                                 data.perc75ExpKappaTotal_perSampleUnbound, data.perc25ExpKappaTotal_perSampleUnbound,
                                                 data.medianExpKappaTotal_perBase_perSampleUnbound);
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

        utils::sampleContainer::iterator boundIt, unboundIt;
		
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
            if((*boundIt).library != 0 && (*boundIt).active)  {
                ++numberOfExp;
                int boundBarcode = (*boundIt).barcode;
                int unboundBarcode = (*unboundIt).barcode;
                utils::countsPerPosPair boundCountsPP;
                utils::countsPerPosPair unboundCountsPP;
                ioTools::readExperimentFile(boundBarcode, expDir+"/2d", boundCountsPP);
                ioTools::readExperimentFile(unboundBarcode, expDir+"/2d", unboundCountsPP);

                //if errors are considered independently for each sample: find respective barcode of wildtype
                int wtBoundBarcode = 0;
                int wtUnboundBarcode = 0;

                    for(auto wtBoundIt=data.bound.begin(); wtBoundIt != data.bound.end(); ++wtBoundIt)
                    {
                        if((*wtBoundIt).library == 0 && (*wtBoundIt).name == (*boundIt).name)
                            wtBoundBarcode = (*wtBoundIt).barcode;
                    }
                    for(auto wtUnboundIt=data.unbound.begin(); wtUnboundIt != data.unbound.end(); ++wtUnboundIt) {
                        if((*wtUnboundIt).library == 0 && (*wtUnboundIt).name == (*unboundIt).name)
                            wtUnboundBarcode = (*wtUnboundIt).barcode;
                    }


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


                        data.mutRateBound_perPos[pos1].resize(3);
                        data.mutRateUnbound_perPos[pos1].resize(3);

                        data.totalRawRelKD_perPos[pos1].resize(3);
                        for(int i=0; i<3; ++i) {
                            data.signal2noiseBound_perPos[pos1][i].reserve(maximalVecSize);
                            data.signal2noiseUnbound_perPos[pos1][i].reserve(maximalVecSize);
                            data.totalRawRelKD_perPos[pos1][i].reserve(maximalVecSize);
                            data.mutRateBound_perPos[pos1][i].reserve(maximalVecSize);
                            data.mutRateUnbound_perPos[pos1][i].reserve(maximalVecSize);
                        }
                    }

                    for(auto pos2It = boundCountsPP[pos1].begin(); pos2It != boundCountsPP[pos1].end(); ++pos2It)
                    {
                        int pos2 = pos2It->first;

                        if(pos2 >= param.seqBegin && pos2 <= param.seqEnd && unboundCountsPP[pos1].find(pos2) != unboundCountsPP[pos1].end()) {
                            utils::Values boundCounts(boundCountsPP[pos1][pos2]);
                            utils::Values unboundCounts(unboundCountsPP[pos1][pos2]);
							
							int wtBase1 = data.ref[pos1];
							int wtBase2 = data.ref[pos2];
							
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

                                    if((param.joinErrors && (data.medianExpKappaBound_perBase.find(pos1) != data.medianExpKappaBound_perBase.end() && data.medianExpKappaBound_perBase[pos1][i]
                                            && data.medianExpKappaUnbound_perBase.find(pos1) != data.medianExpKappaUnbound_perBase.end() && data.medianExpKappaUnbound_perBase[pos1][i]))
                                            || (!param.joinErrors && data.medianExpKappaTotal_perBase_perSampleBound[wtBoundBarcode].find(pos1) != data.medianExpKappaTotal_perBase_perSampleBound[wtBoundBarcode].end()
                                                && data.medianExpKappaTotal_perBase_perSampleBound[wtBoundBarcode][pos1][i]
                                                && data.medianExpKappaTotal_perBase_perSampleUnbound[wtUnboundBarcode].find(pos1) != data.medianExpKappaTotal_perBase_perSampleUnbound[wtUnboundBarcode].end()
                                                && data.medianExpKappaTotal_perBase_perSampleUnbound[wtUnboundBarcode][pos1][i])) {
										
                                        double noiseBound;
                                        double noiseUnbound;

                                        //TODO refactor: error joinen in dem median von allen samples genommen wird
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

                                        data.mutRateBound_perPos[pos1][i].push_back(boundCounts[idx]/boundCountSum);
                                        data.mutRateUnbound_perPos[pos1][i].push_back(unboundCounts[idx]/unboundCountSum);

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

        //TODO: refactor: wirklich correction über ALLE pvalues?
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
                pvaluesIdx[pos1].resize(3, std::numeric_limits<double>::quiet_NaN());
                data.numberOfKDs[pos1].resize(3);
                numberOfKDs_smallerZero[pos1].resize(3);
                numberOfKDs_greaterZero[pos1].resize(3);
                data.pvalues[pos1].resize(3, std::numeric_limits<double>::quiet_NaN());
                data.KDmedians[pos1].resize(3, std::numeric_limits<double>::quiet_NaN());

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

                for(int mnucl=1, mut=0; mnucl<5 && mut<3; ++mnucl) {
                    if(wtBase1 != mnucl) {
                        if(!data.totalRelKD_perPos.at(pos1).at(mut).empty())
                        {
                            for(unsigned int i=0; i <data.totalRelKD_perPos.at(pos1).at(mut).size(); ++i) {
                                  if((data.signal2noiseBound_perPos[pos1][mut][i] < param.minSignal2NoiseStrength
                                    && data.signal2noiseUnbound_perPos[pos1][mut][i] < param.minSignal2NoiseStrength)
                                    || data.positionWeightsBound[pos1][i] < param.weightThreshold || data.numSeqPerPosPairBound[pos1][i] < param.minimumNrCalls
                                    || data.positionWeightsUnbound[pos1][i] < param.weightThreshold || data.numSeqPerPosPairUnbound[pos1][i] < param.minimumNrCalls
                                    || data.mutRateBound_perPos[pos1][mut][i]< std::pow(10,-param.minMutRate)|| data.mutRateUnbound_perPos[pos1][mut][i]< std::pow(10,-param.minMutRate)) {

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
                                    //count number of ressamplings (whicht are not nan) and numbers of binding increasing and decreasing mutations
                                    double KDvalue_log2 = log2(data.totalRelKD_perPos[pos1][mut][i]);
                                    //careful: not all compiler like isnan
                                    if(!std::isnan(KDvalue_log2)) {
                                         validKdsForPercentile[mut].insert(data.totalRelKD_perPos[pos1][mut][i]);
                                        ++data.numberOfKDs[pos1][mut];
                                        //Instead of any signal only consider signals higher or lower a certain threshold as significant
                                        double sigThreshold = param.significanceThreshold == 0 ? 0 : log2(param.significanceThreshold);

                                        if(KDvalue_log2 >= -sigThreshold) {
                                            ++numberOfKDs_greaterZero[pos1][mut];

                                        }
                                        if(KDvalue_log2 <= sigThreshold) {
                                         ++numberOfKDs_smallerZero[pos1][mut];
                                        }
                                    }
                                }
                            }

                            // compute raw pvalues for those where number of evaluable Kds is high enough
                            double median = std::numeric_limits<double>::quiet_NaN();
                            //double perc95;
                            //double perc5;

                            if(data.numberOfKDs[pos1][mut] >= param.minNumberEstimatableKDs) {
                                int numberOfKdsIncludingUpperLower = data.numberOfKDs.at(pos1).at(mut);
                                median = utils::getPercentile(validKdsForPercentile[mut], 50);
                                //perc95 = utils::getPercentile(validKdsForPercentile[mut], 95);
                                //perc5 = utils::getPercentile(validKdsForPercentile[mut], 5);


                                //                        #pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, param, data, mut, pos1, median, perc95, perc5, numberOfKDs_smallerZero, numberOfKDs_greaterZero, lowerBoundIdx)
                                for (unsigned int i = 0; i < lowerBoundIdx[mut].size(); ++i) {
                                    int idx = lowerBoundIdx[mut][i];

                                    if (data.numberOfKDs[pos1][mut] == 0)
                                        data.totalRelKD_perPos[pos1][mut][idx] = std::numeric_limits<double>::quiet_NaN();
                                    else
                                        data.totalRelKD_perPos[pos1][mut][idx] = median;
                                    //                                    else
                                    //                                        data.totalRelKD_perPos[pos1][mut][idx] = perc95;
                                    //count number of resamplings (which are not nan) and numbers of binding increasing and decreasing mutations
                                    double KDvalue_log2 = log2(data.totalRelKD_perPos[pos1][mut][idx]);
                                    //careful: not all compiler like isnan
                                    if (!std::isnan(KDvalue_log2)) {
                                        ++numberOfKdsIncludingUpperLower;
                                        //Instead of any signal only consider signals higher or lower a certain threshold as significant
                                        double sigThreshold = param.significanceThreshold == 0 ? 0 : log2(
                                                param.significanceThreshold);

                                        if (KDvalue_log2 >= -sigThreshold) {
                                            ++numberOfKDs_greaterZero[pos1][mut];

                                        }
                                        if (KDvalue_log2 <= sigThreshold) {
                                            ++numberOfKDs_smallerZero[pos1][mut];
                                        }
                                    }

                                }


                                //                        #pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, param, data, mut, pos1, median, perc95, perc5, numberOfKDs_smallerZero, numberOfKDs_greaterZero, upperBoundIdx)
                                for (unsigned int i = 0; i < upperBoundIdx[mut].size(); ++i) {
                                    int idx = upperBoundIdx[mut][i];

                                    if (data.numberOfKDs[pos1][mut] == 0)
                                        data.totalRelKD_perPos[pos1][mut][idx] = std::numeric_limits<double>::quiet_NaN();
                                    else
                                        data.totalRelKD_perPos[pos1][mut][idx] = median;
                                    //                                    else
                                    //                                        data.totalRelKD_perPos[pos1][mut][idx] = perc5;

                                    //count number of ressamplings (which are not nan) and numbers of binding increasing and decreasing mutations
                                    double KDvalue_log2 = log2(data.totalRelKD_perPos[pos1][mut][idx]);
                                    //careful: not all compiler like isnan
                                    if (!std::isnan(KDvalue_log2)) {
                                        ++numberOfKdsIncludingUpperLower;
                                        //Instead of any signal only consider signals higher or lower a certain threshold as significant
                                        double sigThreshold = param.significanceThreshold == 0 ? 0 : log2(
                                                param.significanceThreshold);

                                        if (KDvalue_log2 >= -sigThreshold) {
                                            ++numberOfKDs_greaterZero[pos1][mut];

                                        }
                                        if (KDvalue_log2 <= sigThreshold) {
                                            ++numberOfKDs_smallerZero[pos1][mut];
                                        }
                                    }

                                }
                                    //collect all  pvalues for Benjamini Hochberg method
                                    pvalues.push_back(min(numberOfKDs_smallerZero[pos1][mut],
                                                          numberOfKDs_greaterZero[pos1][mut]) /
                                                              (double) numberOfKdsIncludingUpperLower);

                                        //rememeber index of pvalue
                                    pvaluesIdx[pos1][mut] = numPValues;
                                    //count number of considered pvalues
                                    ++numPValues;
                            } else {
                                // if not enough evaluable Kds, set everything to NaN
                                pvaluesIdx[pos1][mut] = std::numeric_limits<double>::quiet_NaN();
                                data.totalRelKD_perPos[pos1][mut].clear();

                            }

                            ++mut;
                        }
                    }
                }
            }

		}

		//P-value correction for multiple testing (Benjamini Hochberg method)
		std::vector<size_t> sortedPValueIndices = utils::sort_indexes(pvalues);

		std::vector<bool> pvalueSmallerAlpha(pvalues.size(), false);
		
//		#pragma omp parallel for schedule(guided, 10) default(none) shared(std::cout, param, data, numPValues, pvalues, sortedPValueIndices, pvalueSmallerAlpha)
        for(int i = 0; i < numPValues; ++i) {
                //pvalues[sortedPValueIndices[i]] *=((double)numPValues/(i+1.f));
                //TODO WORKAROUND: take raw pavlues and create a new correction routine. Otherwise clearly signficiant results get kicked out
                //pvalues[sortedPValueIndices[i]] = std::min<double>(1.f,  pvalues[sortedPValueIndices[i]] *((double)numPValues/(i+1.f)));
                //remember pvalues smaller alpha
                pvalueSmallerAlpha[sortedPValueIndices[i]] = pvalues[sortedPValueIndices[i]] < param.alpha;
		}

        if( pvalues.size()> 0) {
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
            } // pvalue index iteration end
        }// end if number of pvalues > 0
        else
        {
            std::cout << "Oh, no evaluable data !" << std::endl;
        }
    } // apply quality criteria end
}
