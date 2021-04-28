#pragma once
// using namespace std;
#include <string>
#include <set>
//#include <boost/regex.hpp>
#include <valarray>
#include <iostream>
#include <vector>
#include <map>
namespace utils {
	
	typedef std::valarray<int> countArray;
	typedef std::valarray<double> rateArray;
	typedef std::map<int, rateArray> countsPerSecondPos;
    typedef std::map<int, std::map<int, rateArray>> countsPerPosPair;
	typedef std::pair<int, int> posPair;
	typedef std::map<int, int> refMap;
    typedef std::map<int, std::vector<double>> ratesPerPos;
    typedef std::map<int, double> ratePerPos;
    typedef std::map<int, std::vector<std::vector<double>>> ratesPerPosPerMut;
	

	enum SampleType {
		
		DNA,
		RNA,
		cDNA,
		BOUND,
		UNBOUND,
		EMPTY
	};
	
	struct Sample {
		
        Sample(int b, SampleType t, int l) : barcode(b), type(t), library(l), active(true) {}
        Sample(std::string n, int b, SampleType t, int l) : name(n), barcode(b), type(t), library(l), active(true)  {}
		
        //name of sampleset (wt + mut bound and unbound)
		std::string name;
		int barcode;
		SampleType type;
        int library;
        //member of a set can not be changed (like const), mutatable allows to change it anyway (if it isn't involved in the sorting comparisson)
        mutable bool active; //if whole sample set is activated in gui (has to be the same for all samples with the same name)
	};

    /* Reorder a vector of counts/ratesfor the opposite position pair  */
    template <typename T>
    T reorderCovariationVector(T& countsVec) {
        T newCountsVec(countsVec.size());
        for(unsigned int i=0; i < countsVec.size(); ++i) {
            int val = (i%4)*4+i/4;
            newCountsVec[val] = countsVec[i];
        }
        return newCountsVec;
    }
	
	
	
	
	class Values {
		private:
			rateArray r;
		public:
			Values(rateArray& rA) : r(rA){}
			
			void setArray(const rateArray& rA) {
				r = rA;
			} 
			
			rateArray getArray() {
				return r;
			}
			
			void addArray(const rateArray& rA) {
				r += rA;
			}
			
			void divide(int n) {
				r /= n;
			}
			
			double sum() {
				return r.sum();
			}
			
			double &operator[](int i)
			{
// 				if( i > r.size())
// 				{
// 				std::cout << "Index out of bounds" << std::endl; 
// 				// return first element.
// 				return r[0];
// 				}
				return r[i];
			}
			
			int size() {
				return r. size();
			}
	};
	
	class mutIdxPerPosPair { 
		
		private:
			std::map<posPair, countArray> mutIdxPerPosPair;
			
		public:
			void addMutIdx(int pos1, int pos2, int idx) {
                std::pair<int, int> p(pos1, pos2);
                if(mutIdxPerPosPair[p].size() == 0) {
                    mutIdxPerPosPair[p].resize(3);
				}
                mutIdxPerPosPair[p] <<= idx;
// 				std::cout << mutIdxPerPosPair[std::make_pair(pos1, pos2)].size() << std::endl;
			}
			
			countArray getValues(int pos1, int pos2) {
				return mutIdxPerPosPair[std::make_pair(pos1, pos2)];
			}
			
			countArray &operator[](posPair p) {
				return mutIdxPerPosPair[p];
			}
	};
	
	class WeightPerPosPair {
		private:
			std::map<posPair, double> weightPerPosPair;
// 			std::map<int, double> maxValPerPos2;
			std::map<int, double> maxValPerPos1;
		public:
			
			void print() {
				for(std::map<posPair,double>::iterator it = weightPerPosPair.begin(); it != weightPerPosPair.end(); ++it ) {
					std::cout << "Pos1 " << (it->first).first << " Pos2 " << (it->first).second << " value " << it->second << std::endl;
				}
			}
			
			void print(int i) {
				for(std::map<posPair,double>::iterator it = weightPerPosPair.begin(); it != weightPerPosPair.end(); ++it ) {
					if((it->first).first == i)
						std::cout << "Pos1 " << (it->first).first << " Pos2 " << (it->first).second << " value " << it->second << std::endl;
				}
			}
			
			void printMaxVals() {
				for(std::map<int, double>::iterator it = maxValPerPos1.begin(); it != maxValPerPos1.end(); ++it) {
					std::cout << "Pos " << it->first << " max Val " << it->second << std::endl;
				}
			}
			
// 			void printMaxVals() {
// 				for(std::map<int, double>::iterator it = totalValPerPos2.begin(); it != totalValPerPos2.end(); ++it) {
// 					std::cout << "Pos " << it->first << " total Val " << it->second << std::endl;
// 				}
// 			}
			
// 			void printTotalVals() {
// 				for(std::map<int, double>::iterator it = maxValPerPos2.begin(); it != maxValPerPos2.end(); ++it) {
// 					std::cout << "Pos " << it->first << " total " << it->second << std::endl;
// 				}
// 			}
// 			
// 			void printMaxVal() {
// 				std::cout << "Max Val: " << totalMaxVal << std::endl;
// 			}
			
			
			void add(int pos1, int pos2,  double r) {
                std::pair<int, int> p(pos1, pos2);
                if(weightPerPosPair.find(p) == weightPerPosPair.end())
                    weightPerPosPair[p] = r;
				else 
                    weightPerPosPair[p] += r;
                double w = weightPerPosPair[p];
// 				if(weightPerPosPair[std::make_pair(pos1, pos2)] > maxValPerPos2[pos2])
// 					maxValPerPos2[pos2] = weightPerPosPair[std::make_pair(pos1, pos2)];
				
                //save maximal seq count for each position (for normalizing)
                if(w > maxValPerPos1[pos1])
                    maxValPerPos1[pos1] = w;

			}

            void put(int pos1, int pos2,  double r) {
                std::pair<int, int> p(pos1, pos2);

                 weightPerPosPair[p] = r;

                double w = weightPerPosPair[p];
// 				if(weightPerPosPair[std::make_pair(pos1, pos2)] > maxValPerPos2[pos2])
// 					maxValPerPos2[pos2] = weightPerPosPair[std::make_pair(pos1, pos2)];

                //save maximal seq count for each position (for normalizing)
                if(w > maxValPerPos1[pos1])
                    maxValPerPos1[pos1] = w;

            }
			
			//divide by the maximum value for all pos2 eg: pos1=1 pos2=3 r=100 and pos1=2 pos2=3 r=20, max value is for pos2=3 is 100
// 			void normalize() {
// 				for(std::map<posPair,double>::iterator it = weightPerPosPair.begin(); it != weightPerPosPair.end(); ++it ) {
// 					int pos = (it->first).second;
// 					it->second /= maxValPerPos2[pos];
// 				}
// 			}
			
			//divide by the maximum value for pos1 eg: pos1=1 pos2=3 r=100 and pos1=1 pos2=4 r=20, max value is for pos2=3 is 100
			void normalize() {
				for(std::map<posPair,double>::iterator it = weightPerPosPair.begin(); it != weightPerPosPair.end(); ++it ) {
					int pos = (it->first).first;
					it->second /= maxValPerPos1[pos];
				}
			}
			
			double getValue(int pos1, int pos2)
			{
				return weightPerPosPair[std::make_pair(pos1, pos2)];
			}
			
			double &operator[](posPair p)
			{
				return weightPerPosPair[p];
			}
			
			int size() {
				return weightPerPosPair.size();
			}
	};
	
	class RatesPerPosPair {
// 		RatesPerPosPair(int pos1, int pos2, vector<double> r): posPair(std::make_pair(pos1, pos2), rate(r) {}
// 		RatesPerPosPair() {}
		private:
			std::map<posPair, rateArray> ratesPerPosPair;
	// 		std::pair<int, int> posPair;
	// 		std::vector<double> rates;
			
		public:
// 			void addPosPair(int pos1, int pos2,  rateArray r) {
// 				ratesPerPosPair[std::make_pair(pos1, pos2)] = r;
// 			}
			
			void print(int pos1, int pos2) {
				rateArray rates = ratesPerPosPair[std::make_pair(pos1, pos2)];
				for(auto it2 = std::begin(rates); it2 != std::end(rates); ++it2) {
					std::cout <<  " value " << *it2 ;
				}
				std::cout << std::endl;
			}
			
			void print() {
				for(std::map<posPair,rateArray>::iterator it = ratesPerPosPair.begin(); it != ratesPerPosPair.end(); ++it ) {
					std::cout << "Pos1 " << (it->first).first << " Pos2 " << (it->first).second << " size "<< (it->second).size() ;
					for(auto it2 = std::begin(it->second); it2 != std::end(it->second); ++it2) {
						std::cout <<  " value " << *it2 ;
					}
					std::cout << std::endl;
				}
			}
			
			void add(int pos1, int pos2,  Values& r) {
                std::pair<int, int> p(pos1, pos2);
				//if no values there before: new entry is created with default value 0 + r
                if(ratesPerPosPair[p].size() == 0) {
                    ratesPerPosPair[p] = r.getArray();
				} else {
                    ratesPerPosPair[p] += r.getArray();
				}
			}

            void put(int pos1, int pos2,  Values& r) {
                std::pair<int, int> p(pos1, pos2);
                ratesPerPosPair[p] = r.getArray();
            }
			
			void divide(int i) {
				for(std::map<posPair, rateArray>::iterator it = ratesPerPosPair.begin(); it != ratesPerPosPair.end(); ++it) {
					it->second /= i;
				}
			}
			
// 			void normalizeRate(int pos1, int pos2, int num) {
// 				ratesPerPosPair[std::make_pair(pos1, pos2)] /= num;
// 			}

			typedef std::map<posPair, rateArray>::iterator iterator;
// 			typedef std::vector<Point>::const_iterator const_iterator;

			iterator begin() { return ratesPerPosPair.begin(); }
			iterator end() { return ratesPerPosPair.end(); }
			
			int getFirstPos(iterator& it) {
				return (it->first).first;
			}
			
			int getSecondPos(iterator& it) {
				return (it->first).second;
			}
			
			posPair getPosPair(iterator& it) {
				return (it->first);
			}
			
			rateArray& getValues(iterator& it) {
				return (it->second);
			}
			
			rateArray& getValues(int pos1, int pos2) {
                //TODO refactor: prüfen ob es existiert, sonst knallts?
                return ratesPerPosPair[std::make_pair(pos1,pos2)];
			}
			
			int size() {
				return ratesPerPosPair.size();
			}
			
			
			
	};
	
    // sort Samples by name, librarynumber and concentration to have the experiment pairs in the same order (TODO: gegeben dass die Proben immer übereinstimmen!)
	struct samplecomp {
		bool operator() (const Sample& lhs, const Sample& rhs) const {
            bool res = lhs.name < rhs.name;
            if( lhs.name == rhs.name)
            {
                res = lhs.library < rhs.library;
                if(lhs.library == rhs.library)
                {
                        res = lhs.barcode < rhs.barcode;
                }
            }
            return res;
		}
	};
	
	typedef std::set<Sample, samplecomp> sampleContainer;
	
    struct Parameter {

        //coeffcient threshold at what a warning is given that the data looks strange
        const double coeffThreshold = 100.0;

        //significant if log2(Kd) is X fold higher/lower than the threshold
        double significanceThreshold = 0;

		//TODO: Enumeration of types
		bool virion = false;

        //if estimated errors differ too much they can also be corrected separately
        bool joinErrors = true;

// 		int cutValueFwd = 30;
        int cutValueFwd = 0;
        int cutValueBwd = 0;
		int seqBegin = cutValueFwd+1;
		int seqEnd;
		
		//threshold of minimum read coverage (enough signal) 
        double weightThreshold = 0.5;
		int minimumNrCalls = 100000;
		
		
		//parameter for result and plot options
		double minSignal2NoiseStrength = 2;
		double alpha = 0.05;
		int minNumberEstimatableKDs = 50;
        //minimum mutation rate (10^-minMutRate)
        double minMutRate = 4.0;

        //Plot values for the KD plots
		int plotStartRegion = 1;
		int plotEndRegion = 110;

        int plotYAxisFrom = -4;
        int plotYAxisTo = 4;

        bool plotPValues = true;
		// replace lower bounds for KDvalues  with median, otherwise with 5th and 95th percentile
//		bool putMedian = true;

		std::string refFile;
        std::string dataDir;
        std::string resultDir;

	};
	
	struct DataContainer {
		
		//reference sequence (1. pos, 2. A=1, C=2, G=3, T=4)
		refMap ref;
		
        //TODO refactor: dna weg
		//barcodes for wildtype dna, bound and unbound libraries
		sampleContainer dna, bound, unbound;

        //map barcode with sampleFile (for loading, reading and saving of those)
        std::map<int, std::string> sampleFiles;

		std::vector<int> positions;

        //coefficient of variation of errors is only plotted. but there should be a warning if the coeff. is too high so the errors should be considered separately
        //(after plotting the errors and going to the step of KD computation)
        double maxCoVarErrorBound;
        double maxCoVarErrorUnbound;
		
		//expected noise 
		std::map<int, std::vector<double>> medianExpKappaBound_perBase;
		std::map<int, std::vector<double>> medianExpKappaUnbound_perBase;
//		std::map<int, double> medianExpKappa_Total;
        //expected errors per sample for each mutation (for seperately corrected errors)
        std::map<int, std::map<int, std::vector<double>>> medianExpKappaTotal_perBase_perSampleBound;
        std::map<int, std::map<int, std::vector<double>>> medianExpKappaTotal_perBase_perSampleUnbound;
        //expected errors per sample (so far only used for plot, not for computation
		std::map<int, std::map<int, double>> medianExpKappaTotal_perSampleBound;
		std::map<int, std::map<int, double>> medianExpKappaTotal_perSampleUnbound;
        //quartile errors per sample (so far only used for plot, not for computation
        std::map<int, std::map<int, double>> perc75ExpKappaTotal_perSampleBound;
        std::map<int, std::map<int, double>> perc75ExpKappaTotal_perSampleUnbound;
        std::map<int, std::map<int, double>> perc25ExpKappaTotal_perSampleBound;
        std::map<int, std::map<int, double>> perc25ExpKappaTotal_perSampleUnbound;

        //mean and std expected errors per sample (so far only used for plot, not for computation)
//        std::map<int, std::map<int, double>> meanExpKappaTotal_perSampleBound;
//        std::map<int, std::map<int, double>> meanExpKappaTotal_perSampleUnbound;
//        std::map<int, std::map<int, double>> stdExpKappaTotal_perSampleBound;
//        std::map<int, std::map<int, double>> stdExpKappaTotal_perSampleUnbound;



        //for each position collect all  mutation rates per mutationsites(3) in 2 vectors
        std::map<int, std::vector<std::vector<double>>> mutRateBound_perPos;
        std::map<int, std::vector<std::vector<double>>> mutRateUnbound_perPos;
		
		// for each position collect all  singal2noise values per mutationsites(3) in 2 vectors
		std::map<int, std::vector<std::vector<double>>> signal2noiseBound_perPos;
		std::map<int, std::vector<std::vector<double>>> signal2noiseUnbound_perPos;
		
        // relative raw KD values per position, for all experiments in one vector (for all 3 mutations)
        std::map<int, std::vector<std::vector<double>>> totalRawRelKD_perPos;
        // relative KD values per position, for all experiments in one vector (for all 3 mutations), after application of quality criteria
        std::map<int, std::vector<std::vector<double>>> totalRelKD_perPos;
		
		//weights for each pair of positions for each experiment pair
		std::map<int, std::vector<double>> positionWeightsBound;
		std::map<int, std::vector<double>> positionWeightsUnbound;
//		std::map<int, std::vector<double>> positionWeightsTotal;
		
		std::map<int, std::vector<double>> numSeqPerPosPairBound;
		std::map<int, std::vector<double>> numSeqPerPosPairUnbound;
//		std::map<int, std::vector<double>> numSeqPerPosPairTotal;
		
		//pvalues per position and mutation
		std::map<int, std::vector<double>> pvalues;
		//count number of samples which are valid (not nan)
		std::map<int, std::vector<int>> numberOfKDs;
		//mutation with maximal effect
		std::map<int, int> maxMut;
		//number of KD estimates with lower and upper estimates
		std::map<int, std::vector<int>> lowerLimitsKD_perPos;
		std::map<int, std::vector<int>> upperLimitsKD_perPos;
		std::map<int, std::vector<double>> KDmedians;

        //WT error for correction for in virions experiments (not ligated vectors)
        std::map<int, double> wtErrors;

        void activateSamples(std::string name, bool activate) {
            for(auto& s : bound) {
                if(s.name == name)
                     s.active = activate;
            }
            std::cout << std::endl;

            for(auto& s : unbound) {
                if(s.name == name)
                    s.active = activate;
            }

            for(auto& s : dna) {
                if(s.name == name)
                    s.active = activate;
            }
        }

        bool errorExist() {
            return (!medianExpKappaBound_perBase.empty() &&
                    !medianExpKappaUnbound_perBase.empty() &&
                    !medianExpKappaTotal_perSampleBound.empty() &&
                    !medianExpKappaTotal_perSampleUnbound.empty() &&
                    !medianExpKappaTotal_perBase_perSampleBound.empty() &&
                    !medianExpKappaTotal_perBase_perSampleUnbound.empty());
        }

        //clear computed errors
        void clearErrors() {
            medianExpKappaBound_perBase.clear();
            medianExpKappaUnbound_perBase.clear();
            medianExpKappaTotal_perSampleBound.clear();
            medianExpKappaTotal_perSampleUnbound.clear();
            medianExpKappaTotal_perBase_perSampleBound.clear();
            medianExpKappaTotal_perBase_perSampleUnbound.clear();
//            medianExpKappa_Total.clear();
        }

        //clear computed cirteria for raw KD values
        void clearSignal2Noise() {
            signal2noiseBound_perPos.clear();
            signal2noiseUnbound_perPos.clear();
//            signal2noiseBound_total.clear();
//            signal2noiseUnbound_total.clear();
        }

        void clearMutRates() {
            mutRateBound_perPos.clear();
            mutRateUnbound_perPos.clear();
        }

        void clearPositionWeights() {
            positionWeightsBound.clear();
            positionWeightsUnbound.clear();
//            positionWeightsTotal.clear();
        }

        void clearNumSeq() {
            numSeqPerPosPairBound.clear();
            numSeqPerPosPairUnbound.clear();
//            numSeqPerPosPairTotal.clear();

        }

        void clearRawKDCriteria() {
            clearMutRates();
            clearNumSeq();
            clearPositionWeights();
            clearSignal2Noise();
            totalRawRelKD_perPos.clear();
            totalRelKD_perPos.clear();
            wtErrors.clear();
        }

        //clear results with applied quality criteria
        void clearKDQualityCriteria() {
            //std::cout << "clear pvalues ";
            pvalues.clear();
           // std::cout << " clear numKd ";
            numberOfKDs.clear();
           // std::cout << " clear kd medians ";
            KDmedians.clear();
          //  std::cout << " lower upper ";
            lowerLimitsKD_perPos.clear();
            upperLimitsKD_perPos.clear();
         //   std::cout << " total rel ";
            totalRelKD_perPos.clear();
          //  std::cout << " pos ";
            positions.clear();
          //  std::cout << " maxmut ";
            maxMut.clear();
        }
		
	};
	
	
//	const boost::regex pattern("tg([[:digit:]]+).*.npy.txt", boost::regex::extended);
//	const boost::regex pattern2("tg([[:digit:]]+).*.txt", boost::regex::extended);
//    const boost::regex pattern3("([[:digit:]]+).*.txt", boost::regex::extended);


// 	typedef std::map<std::pair<int, int>, std::vector<double>> ratesPerPosPair;
	
    typedef std::vector<std::vector<std::vector<int>>> nuclCountMatrix;
	typedef std::vector<std::vector<std::vector<double>>> nuclRateMatrix;
	
	SampleType getSampleTypeFromString(std::string& s);

    double getMean(std::valarray<double>& v);

    double getStd(std::valarray<double>& v);

    double getCoeffVar(std::valarray<double>& v);

	double getMedian(std::multiset<double>& s);
	
	double getPercentile(std::multiset<double>& s, int p);
	
// 	template <typename T>
	double getPercentile(std::vector<double>& values, int percentile);

    std::pair<double, double> getxIQR(std::vector<double>& values);
	
// 	template <typename T>
	std::vector<size_t> sort_indexes(std::vector<double>& v);
	
}
