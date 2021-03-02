#include "ioTools.hpp"

#include <algorithm>
#include<filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <mimeexception.hpp>
namespace fs = std::filesystem;

namespace ioTools {

    std::vector<std::string> split(const std::string& s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string createDir(const std::string& resDir) {
        fs::path resultDirPath(resDir);
        if(!fs::is_directory(resultDirPath))
            fs::create_directory(resultDirPath);
        //return matching string for the platform
        return resultDirPath.string();
    }

    void printMapOfVector(utils::ratesPerPos& mapOfVector, const string& filename, const string& resultDir) {
        fs::path file(resultDir +"/"+ filename);
        std::ofstream outfile(file.string());
        if(outfile.good()) {
            for(utils::ratesPerPos::iterator posIt = mapOfVector.begin(); posIt != mapOfVector.end(); ++posIt) {
                outfile << posIt->first;
                for(auto valIt = (posIt->second).begin(); valIt != (posIt->second).end(); ++valIt) {
                    outfile << "\t" <<  std::setprecision(20) << *valIt;
                }
                outfile << std::endl;
            }
        }
        outfile.close();
    }

    void printMapOfDouble(utils::ratePerPos& ratePerPos, const string& filename, const string& resultDir) {
        fs::path file(resultDir +"/"+ filename);
        std::ofstream outfile(file.string());
        if(outfile.good()) {
            for(utils::ratePerPos::iterator posIt = ratePerPos.begin(); posIt != ratePerPos.end(); ++posIt) {
                outfile << posIt->first << "\t" << posIt->second << std::endl;
            }
        }
        outfile.close();
    }

    void printMapOfVectorOfVector(utils::ratesPerPosPerMut& ratesPerPosPerMut, const string& filename, const string& resultDir) {
        fs::path file(resultDir +"/"+ filename);
        std::ofstream outfile(file.string());
        if(outfile.good()) {
            for(utils::ratesPerPosPerMut::iterator posIt = ratesPerPosPerMut.begin(); posIt != ratesPerPosPerMut.end(); ++posIt) {
                for(int i=0; i<3; ++i) {
                    outfile << posIt->first << "\t" << i;
                    for(auto mutIt = (posIt->second)[i].begin(); mutIt != (posIt->second)[i].end(); ++mutIt) {
                        if(!std::isnan(*mutIt))
                            outfile << "\t" << std::setprecision(10) << *mutIt;
                    }
                    outfile << std::endl;
                }

            }

        }
        outfile.close();
    }


    //write all caught exceptions and other messages into a log file
    void writeErrorLog(const string& resultDir, const std::string& text)
    {
        createDir(resultDir+"/tmp");
        fs::path errorLogFile(resultDir+"/tmp/error.log");
        std::ofstream outfile(errorLogFile.string(), std::ofstream::out | std::ofstream::app);
        if(outfile.good()) {
                outfile << text << std::endl;
        }
        outfile.close();
    }

    string readReference(const string& refStr, utils::refMap& ref) {
		
        //utils::refMap ref;
		
        //clear old content of reference sequence
        ref.clear();

        fs::path refFile(refStr);
        if(fs::exists(refFile) && fs::is_regular_file(refFile)) {
            std::cout << "Read reference file..." << std::endl;
            std::ifstream infile(refFile.string());
            if (infile.good()) {
                string line;
                istringstream ss;
                //allow differnt formats for reference file: either fasta, or number 1-4 for base in the format pos,base. if header > is present, it's fasta
                getline(infile, line);
                bool isFasta = (line.find(">") == 0);
                int position = 1;
                bool end = false;
                while(!infile.eof() && !end) {

                    if(isFasta)
                    {
                       getline(infile, line);
                       ss.clear();
                       ss.str(line);
                       end = (line.find(">") == 0);
                       if(!end)
                       {
                           char c;
                           while(ss.get(c))
                           {
                               if(c=='A' || c=='a')
                                   ref[position] = 1;
                               else if(c=='C' || c=='c')
                                   ref[position] = 2;
                               else if(c=='G' || c=='g')
                                   ref[position] = 3;
                               else if(c=='T' || c=='t' || c=='U' || c=='u')
                                   ref[position] = 4;
                               else //ignore white space and line breaks
                                   continue;
                               ++position;
                           }
                       }
                    } else {
                        //because first position is read above
                        if(position != 1)
                            getline(infile, line);
                        if(line.size()==0)
                            continue;
                        ss.clear();
                        ss.str(line);
                        // first element is position in reference
                        string pos;
                        //second element is number for base
                        string base;
                        getline(ss, pos, ',');
                        getline(ss, base);
                        ref[stoi(pos)] = stoi(base);
                        ++position;
                    }
                }
                infile.close();
            } else {
                throw MIME_NoSuchFileException(refFile.string());
            }

        } else {
            throw MIME_NoSuchFileException(refFile.string());
        }
        return refFile.string();
	}



    //reading counts from one file
    void readExperimentFile(int barcode, const string& expDir, utils::countsPerPosPair& countsPP) {
        string bc = std::to_string(barcode);
        fs::path file(expDir+"/"+bc+".txt");
        if(fs::exists(file) && fs::is_regular_file(file))
        {
            std::ifstream infile(file.string());
            if (infile.good()) {
                string line;
                //Header
                getline(infile, line);
                istringstream ss;
                while(getline(infile, line)) {
//                    vector<string> splittedLine;
                    if(line.size()==0)
                        continue;
                    ss.clear();
                    ss.str(line);
                    // first two elements are the positions
                    string pos1, pos2;
                    getline(ss, pos1, '\t');
                    getline(ss, pos2, '\t');

                    //read the read counts for the pair of psoitions
                    std::vector<std::string> countsVec(istream_iterator<string>{ss}, istream_iterator<string>{});
                    utils::rateArray counts(countsVec.size());
                    std::transform(countsVec.begin(), countsVec.end(), std::begin(counts), [](const auto& val)
                    {
                        return std::stod(val);
                    });

//                    pair<int, int> posPair1(stoi(pos1), stoi(pos2));
//                    pair<int, int> posPair2(stoi(pos2), stoi(pos1));

                   countsPP[std::stoi(pos1)][std::stoi(pos2)] = counts;
                   countsPP[stoi(pos2)][stoi(pos1)]= utils::reorderCovariationVector<utils::rateArray>(counts);
                }
            } else {
                throw MIME_NoSuchFileException(file.string());
            }
        } else {

            throw MIME_NoSuchFileException(file.string());
        }
    }

	
    void readExperimentFile(int barcode, const string& expDir, int pos, utils::countsPerSecondPos& countsPP) {
		string bc = std::to_string(barcode);
		string p = std::to_string(pos);
		fs::path posFile(expDir+"/"+bc+"/"+bc+"_"+p+".txt");
		if(fs::exists(posFile) && fs::is_regular_file(posFile)) {
            std::ifstream infile(posFile.string());
			if (infile.good()) {
				string line;
// 				istringstream ss;
				while(getline(infile, line)) {
					vector<string> splittedLine =  split(line, ',');
					string pos2 = splittedLine[2];

					// +4 to skip both positions and nucleotides
					vector<string> countsStr(splittedLine.begin()+4, splittedLine.end());
					utils::rateArray counts;
					counts.resize(countsStr.size());
                    std::transform(countsStr.begin(), countsStr.end(), std::begin(counts), [](const auto& val)
                    {
                        return std::stod(val);
                    });
					countsPP[stoi(pos2)] = counts;
					
				}
			} else {
                throw MIME_NoSuchFileException(posFile.string());
			}
		} else {
            throw MIME_NoSuchFileException(posFile.string());
		}
	}

    void readExperimentFileAndSum(int barcode, const string& expDir, std::map<int, utils::rateArray>& counts) {
        string bc = std::to_string(barcode);
        fs::path posFile(expDir+"/"+bc+".txt");
//        fs::path posFile(expDir+"/tg"+bc+"_1D.txt");
        if(fs::exists(posFile) && fs::is_regular_file(posFile)) {
            std::ifstream infile(posFile.string());
            if (infile.good()) {
                string line;
                //first line = header
                getline(infile, line);
                int actPos = 0;
//                 while(getline(infile, line) && pos!=actPos) {
                while(getline(infile, line)) {

                    vector<string> splittedLine = split(line, '\t');
                    actPos = std::stoi(splittedLine[0]);
                    counts[actPos].resize(4);

                // +1 to skip the position
                    std::transform(splittedLine.begin()+1, splittedLine.end(), std::begin(counts[actPos]), [](const auto& val)
                    {
                        return std::stod(val);
                    });
                }
            } else {
                throw MIME_NoSuchFileException(posFile.string());
            }
        } else {
            throw MIME_NoSuchFileException(posFile.string());
        }
    }
    
    void readAndCompute1dMutationRate(int barcode, const string& expDir, utils::DataContainer& data, std::vector<double>& rates) {
        string bc = std::to_string(barcode);
        fs::path posFile(expDir+"/"+bc+".txt");;
        if(fs::exists(posFile) && fs::is_regular_file(posFile)) {
            std::ifstream infile(posFile.string());
            if (infile.good()) {
                string line;
                //first line = header
                getline(infile, line);
                int actPos = 0;
                while(getline(infile, line)) {

                    vector<string> splittedLine = ioTools::split(line, '\t');
                    actPos = std::stoi(splittedLine[0]);
                    int wt = data.ref[actPos];

                    utils::rateArray counts(4);
                    std::transform(splittedLine.begin()+1, splittedLine.end()+5, std::begin(counts), [](const auto& val)
                    {
                        return std::stod(val);
                    });

                    double mutSum = 0.0;
                    for(int mut=0; mut < 3; ++mut) {
                    if(mut+1 != wt)
                        mutSum += counts[mut];
                    }
                    rates.push_back(mutSum/counts.sum());
                }
            } else {
                throw MIME_NoSuchFileException(posFile.string());
            }
        } else {
            throw MIME_NoSuchFileException(posFile.string());
        }
    }

    /*
    void writeErrorEstimates(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Write Error Estimates... " << std::endl;
        std::string errorDir = ioTools::createDir(resultDir+"/errors");
        ioTools::printMapOfVector(data.medianExpKappaBound_perBase, "errorEstimatesPerBaseSelected.csv", errorDir);
        ioTools::printMapOfVector(data.medianExpKappaUnbound_perBase, "errorEstimatesPerBaseNonselected.csv", errorDir);
//        printMapOfDouble(data.medianExpKappa_Total, "errorEstimatesTotal.csv", resultDir);
//        std::map<int, std::map<int, double>> medianExpKappaTotal_perSampleSelected;

        for(auto sampleUnboundIt = data.medianExpKappaTotal_perSampleUnbound.begin(); sampleUnboundIt!=data.medianExpKappaTotal_perSampleUnbound.end(); ++sampleUnboundIt) {
            int unboundBarcode = sampleUnboundIt->first;
            ioTools::printMapOfDouble(data.medianExpKappaTotal_perSampleUnbound[unboundBarcode], "errorEstimatesTotal_nonselectedSample"+std::to_string(unboundBarcode)+".csv", errorDir);
            ioTools::printMapOfVector(data.medianExpKappaTotal_perBase_perSampleUnbound[unboundBarcode], "errorEstimatesTotalPerBase_nonselectedSample"+std::to_string(unboundBarcode)+".csv", errorDir);

            ioTools::printMapOfDouble(data.perc25ExpKappaTotal_perSampleUnbound[unboundBarcode], "perc25ErrorTotal_nonselectedSample"+std::to_string(unboundBarcode)+".csv", errorDir);
            ioTools::printMapOfDouble(data.perc75ExpKappaTotal_perSampleUnbound[unboundBarcode], "perc75ErrorTotal_nonselectedSample"+std::to_string(unboundBarcode)+".csv", errorDir);
        }

        for(auto sampleBoundIt = data.medianExpKappaTotal_perSampleBound.begin(); sampleBoundIt!=data.medianExpKappaTotal_perSampleBound.end(); ++sampleBoundIt) {
            int boundBarcode = sampleBoundIt->first;
            printMapOfDouble(data.medianExpKappaTotal_perSampleBound[boundBarcode], "errorEstimatesTotal_selectedSample"+std::to_string(boundBarcode)+".csv", errorDir);
            printMapOfVector(data.medianExpKappaTotal_perBase_perSampleBound[boundBarcode], "errorEstimatesTotalPerBase_selectedSample"+std::to_string(boundBarcode)+".csv", errorDir);

            printMapOfDouble(data.perc25ExpKappaTotal_perSampleBound[boundBarcode], "perc25ErrorTotal_selectedSample"+std::to_string(boundBarcode)+".csv", errorDir);
            printMapOfDouble(data.perc75ExpKappaTotal_perSampleBound[boundBarcode], "perc75ErrorTotal_selectedSample"+std::to_string(boundBarcode)+".csv", errorDir);
        }
    }
    */

    void readRatesPerPosition(utils::ratesPerPos& rates, const string& filename, const string& resultDir) {
        fs::path file(resultDir +"/"+ filename);

	    if(fs::exists(file) && fs::is_regular_file(file)) {
            std::ifstream infile(file.string());
            if (infile.good()) {
                rates.clear();
                string line;
                while(getline(infile, line)) {

                    vector<string> splittedLine = ioTools::split(line, '\t');
                    int pos = std::stoi(splittedLine[0]);
                    // +1 to skip both positions
                    vector<string> countsStr(splittedLine.begin()+1, splittedLine.end());
                    rates[pos].resize(countsStr.size());
                    std::transform(countsStr.begin(), countsStr.end(), std::begin(rates[pos]), [](const auto& val)
                    {
                        return std::stod(val);
                    });
                }
            } else {
                throw MIME_NoSuchFileException(file.string());
            }
        } else {
            throw MIME_NoSuchFileException(file.string());
        }
    }
    

    void readDoublePerPosition(utils::ratePerPos& errors, const string& filename, const string& resultDir) {
        fs::path file(resultDir +"/"+ filename);

       if(fs::exists(file) && fs::is_regular_file(file)) {
            std::ifstream infile(file.string());
            if (infile.good()) {
                errors.clear();
                string line;
                while(getline(infile, line)) {

                    vector<string> splittedLine = ioTools::split(line, '\t');
                    int pos = std::stoi(splittedLine[0]);
                    double value = std::stod(splittedLine[1]);
                    errors[pos] = value;
                }
            } else {
                throw MIME_NoSuchFileException(file.string());
            }
        } else {
           throw MIME_NoSuchFileException(file.string());
        } 
    }

    void readVectorOfVectorPerPosition(utils::ratesPerPosPerMut& rates, const string& filename, const string& resultDir) {
        fs::path file(resultDir +"/"+ filename);
        if(fs::exists(file) && fs::is_regular_file(file)) {
            std::ifstream infile(file.string());
            rates.clear();
            if(infile.good()) {
                string line;
                while(getline(infile, line)) {
                    vector<string> splittedLine = ioTools::split(line, '\t');
                    int pos1 = std::stoi(splittedLine[0]);
                    int mut = std::stoi(splittedLine[1]);
                    //the vector has to be newly defined
                    if(mut == 0)
                        rates[pos1].resize(3);
                    (rates[pos1][mut]).resize(splittedLine.size()-2);
                    std::transform(splittedLine.begin()+2, splittedLine.end(), (rates[pos1][mut]).begin(), [](const auto& val)
                    {
                        return std::stod(val);
                    });
                }
            }
        }
    }
    
    void readErrorEstimates(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Read Error Estimates... " << std::endl;
        std::string errorDir(resultDir+"/errors");
        readRatesPerPosition(data.medianExpKappaBound_perBase, "errorEstimatesPerBaseSelected.csv", errorDir);
        readRatesPerPosition(data.medianExpKappaUnbound_perBase, "errorEstimatesPerBaseNonselected.csv", errorDir);
        for(auto boundIt=data.bound.begin(), unboundIt=data.unbound.begin(); boundIt != data.bound.end(); ++boundIt, ++unboundIt) {
            if((*boundIt).library == 0)
            {
                int boundBarcode = (*boundIt).barcode;
                int unboundBarcode = (*unboundIt).barcode;
                std::string fileName = "errorEstimatesTotal_selectedSample"+std::to_string(boundBarcode)+".csv";
                readDoublePerPosition(data.medianExpKappaTotal_perSampleBound[boundBarcode], fileName, errorDir);
                fileName = "errorEstimatesTotal_nonselectedSample"+std::to_string(unboundBarcode)+".csv";
                readDoublePerPosition(data.medianExpKappaTotal_perSampleUnbound[unboundBarcode], fileName, errorDir);

                fileName = "errorEstimatesTotalPerBase_selectedSample"+std::to_string(boundBarcode)+".csv";
                std::map<int, std::vector<double>> rates;
                readRatesPerPosition(rates, fileName, errorDir);
                data.medianExpKappaTotal_perBase_perSampleBound[boundBarcode] = rates;

                rates.clear();
                fileName = "errorEstimatesTotalPerBase_nonselectedSample"+std::to_string(unboundBarcode)+".csv";
                readRatesPerPosition(rates, fileName, errorDir);
                data.medianExpKappaTotal_perBase_perSampleUnbound[unboundBarcode] = rates;
                rates.clear();


                fileName = "perc25ErrorTotal_selectedSample"+std::to_string(boundBarcode)+".csv";
                readDoublePerPosition(data.perc25ExpKappaTotal_perSampleBound[boundBarcode], fileName, errorDir);
                fileName = "perc25ErrorTotal_nonselectedSample"+std::to_string(unboundBarcode)+".csv";
                readDoublePerPosition(data.perc25ExpKappaTotal_perSampleUnbound[unboundBarcode], fileName, errorDir);
                fileName = "perc75ErrorTotal_selectedSample"+std::to_string(boundBarcode)+".csv";
                readDoublePerPosition(data.perc75ExpKappaTotal_perSampleBound[boundBarcode], fileName, errorDir);
                fileName = "perc75ErrorTotal_nonselectedSample"+std::to_string(unboundBarcode)+".csv";
                readDoublePerPosition(data.perc75ExpKappaTotal_perSampleUnbound[unboundBarcode], fileName, errorDir);
            }
        }
    }


	/*
    void writeRawKDValues(const string& resultDir, utils::DataContainer& data, const string& filename) {
        std::cout << "Write raw KD values... " << std::endl;
        printMapOfVectorOfVector(data.totalRawRelKD_perPos, filename, resultDir);
	}

	
	void readRawKDValues(const string& resultDir, utils::DataContainer& data, const string& filename) {
		std::cout << "Read raw KD values... " << std::endl;
        readVectorOfVectorPerPosition(data.totalRawRelKD_perPos, filename, resultDir);
	}



    void writeSignal2Noise(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Write signal to noise ratio... " << std::endl;
        ioTools::printMapOfVectorOfVector(data.signal2noiseBound_perPos, "signal2NoiseRatioSelected.csv", resultDir);
        ioTools::printMapOfVectorOfVector(data.signal2noiseUnbound_perPos, "signal2NoiseRatioNonselected.csv", resultDir);
    }

    void readSignal2Noise(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Read signal to noise ratio... " << std::endl;
        readVectorOfVectorPerPosition(data.signal2noiseBound_perPos, "signal2NoiseRatioSelected.csv", resultDir);
        readVectorOfVectorPerPosition(data.signal2noiseUnbound_perPos, "signal2NoiseRatioNonselected.csv", resultDir);
    }

    void writeMutRate(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Write mutation rates... " << std::endl;
        ioTools::printMapOfVectorOfVector(data.mutRateBound_perPos, "mutRatesSelected.csv", resultDir);
        ioTools::printMapOfVectorOfVector(data.mutRateUnbound_perPos, "mutRatesNonselected.csv", resultDir);
    }

    void readMutRate(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Read mutation rates... " << std::endl;
        readVectorOfVectorPerPosition(data.mutRateBound_perPos, "mutRatesSelected.csv", resultDir);
        readVectorOfVectorPerPosition(data.mutRateUnbound_perPos, "mutRatesNonselected.csv", resultDir);
    }

    void writePositionWeights(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Write percentage of maximal coverage... " << std::endl;
        printMapOfVector(data.positionWeightsBound, "percentOfMaxCovSelected.csv", resultDir);
        printMapOfVector(data.positionWeightsUnbound, "percentOfMaxCovNonselected.csv", resultDir);
    }

    void readPositionWeights(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Read percentage of maximal coverage... " << std::endl;
        readRatesPerPosition(data.positionWeightsBound, "percentOfMaxCovSelected.csv", resultDir);
        readRatesPerPosition(data.positionWeightsUnbound, "percentOfMaxCovNonselected.csv", resultDir);
    }


    void writeSequenceNumbers(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Write coverage... " << std::endl;
        printMapOfVector(data.numSeqPerPosPairBound, "coverageSelected.csv", resultDir);
        printMapOfVector(data.numSeqPerPosPairUnbound, "coverageNonselected.csv", resultDir);
    }


    void readSequenceNumbers(const string& resultDir, utils::DataContainer& data) {
        std::cout << "Read coverage... " << std::endl;
        readRatesPerPosition(data.numSeqPerPosPairBound, "coverageSelected.csv", resultDir);
        readRatesPerPosition(data.numSeqPerPosPairUnbound, "coverageNonselected.csv", resultDir);
    }


    void writeRawKDCriteria(const string& resDir, utils::DataContainer& data, const string& filename) {
        std::string resultDir = createDir(resDir+"/KdResults");
        writeRawKDValues(resultDir, data, filename);
        writeMutRate(resultDir, data);
        writeSignal2Noise(resultDir, data);
        writePositionWeights(resultDir, data);
        writeSequenceNumbers(resultDir, data);
    }



    void readRawKDCriteria(const string& resDir, utils::DataContainer& data, const string& filename) {
        std::string resultDir(resDir+"/KdResults");
        data.clearRawKDCriteria();
        readRawKDValues(resultDir, data, filename);
        readMutRate(resultDir, data);
        readSignal2Noise(resultDir, data);
        readPositionWeights(resultDir, data);
        readSequenceNumbers(resultDir, data);
    }
    */

	
    std::string writePositionWiseKDEstimates(const string &resDir, utils::DataContainer& data, const string &filenamePrefix) {
        std::cout << "Write positionwise KD estimates... " << std::endl;
        std::string filename = "PositionWiseKdEstimates";
        if(!filenamePrefix.empty())
            filename += "_";
        fs::path resultDir(resDir);
        if(!fs::is_directory(resultDir))
            fs::create_directory(resultDir);
        fs::path KDFile(resultDir.string()+"/"+filename+filenamePrefix+".csv");
        std::ofstream outfile(KDFile.string());
		if(outfile.good()) {
            outfile << "pos1\twt base\tmax effect mut";
            outfile << "\tmedian mut A\tp-values mut A\t#resamplings mut A\t#lower estimates mut A\t#upper estimates mut A\t5. percentil mut A\t95. percentil mut A";
            outfile << "\tmedian mut C\tp-values mut C\t#resamplings mut C\t#lower estimates mut C\t#upper estimates mut C\t5. percentil mut C\t95. percentil mut C";
            outfile << "\tmedian mut G\tp-values mut G\t#resamplings mut G\t#lower estimates mut G\t#upper estimates mut G\t5. percentil mut G\t95. percentil mut G";
            outfile << "\tmedian mut U\tp-values mut U\t#resamplings mut U\t#lower estimates mut U\t#upper estimates mut U\t5. percentil mut U\t95. percentil mut U" << std::endl;
            //for(auto posIt = data.totalRelKD_perPos.begin(); posIt != data.totalRelKD_perPos.end(); ++posIt) {
            for(auto posIt = data.positions.begin(); posIt != data.positions.end(); ++posIt) {
                int pos1 = *posIt;
                //int pos1 = posIt->first;
				int wtBase1 = data.ref[pos1];
				outfile << pos1 << "\t" << wtBase1 << "\t" << data.maxMut[pos1];

				for(int mnucl=1, mut=0; mnucl<5; ++mnucl) {
					if(wtBase1 != mnucl) {
                        outfile << "\t" << std::setprecision(10) <<  data.KDmedians[pos1][mut];
                        outfile << "\t" << std::setprecision(10) << data.pvalues[pos1][mut];
                        outfile << "\t" << std::setprecision(10) << data.numberOfKDs[pos1][mut];
                        outfile << "\t" << std::setprecision(10) << data.lowerLimitsKD_perPos[pos1][mut];
                        outfile << "\t" << std::setprecision(10) << data.upperLimitsKD_perPos[pos1][mut];
                        outfile << "\t" << std::setprecision(10) << utils::getPercentile(data.totalRelKD_perPos[pos1][mut], 5);
                        outfile << "\t" << std::setprecision(10) << utils::getPercentile(data.totalRelKD_perPos[pos1][mut], 95);
						++mut;
                    } else {
                        outfile << "\t" << std::numeric_limits<double>::quiet_NaN();
                        outfile << "\t" << std::numeric_limits<double>::quiet_NaN();
                        outfile << "\t" << std::numeric_limits<double>::quiet_NaN();
                        outfile << "\t" << std::numeric_limits<double>::quiet_NaN();
                        outfile << "\t" << std::numeric_limits<double>::quiet_NaN();
                        outfile << "\t" << std::numeric_limits<double>::quiet_NaN();
                        outfile << "\t" << std::numeric_limits<double>::quiet_NaN();
                    }
				}
				outfile << std::endl;
			}	
        } else {
            throw MIME_NoSuchFileException(KDFile.string());
        }
		outfile.close();
        return KDFile.string();
	}

    std::string writePositionWiseMaxKD(const string &resDir, utils::DataContainer& data, const string &filenamePrefix) {
        std::cout << "Write positionwise maximum KD estimates... " << std::endl;
        std::string filename = "PositionWiseMaxKd";
        if(!filenamePrefix.empty())
            filename += "_";
        fs::path resultDir(resDir);
        if(!fs::is_directory(resultDir))
            fs::create_directory(resultDir);
        fs::path KDFile(resultDir.string()+"/"+filename+filenamePrefix+".csv");
        std::ofstream outfile(KDFile.string());
        if(outfile.good()) {
            outfile << "pos1\twt base\tmax effect mut";
            outfile << "\tmedian Kd\tp-value\t#resamplings\t#lower estimates\t#upper estimates\t5. percentil\t95. percentil\t25. percentil\t75. percentil" << std::endl;
            for(auto posIt = data.totalRelKD_perPos.begin(); posIt != data.totalRelKD_perPos.end(); ++posIt) {
                int pos1 = posIt->first;
                int wtBase1 = data.ref[pos1];
                int maxmut = data.maxMut[pos1];
                int maxmutIdx = maxmut-1;
                if(!std::isnan(maxmut) && maxmut > 0) {
                    if(maxmut > data.ref[pos1])
                        --maxmutIdx;
                    outfile << pos1 << "\t" << wtBase1 << "\t" << data.maxMut[pos1];
                    outfile << "\t" << std::setprecision(10) <<  data.KDmedians[pos1][maxmutIdx];
                    outfile << "\t" << std::setprecision(10) << data.pvalues[pos1][maxmutIdx];
                    outfile << "\t" << std::setprecision(10) << data.numberOfKDs[pos1][maxmutIdx];
                    outfile << "\t" << std::setprecision(10) << data.lowerLimitsKD_perPos[pos1][maxmutIdx];
                    outfile << "\t" << std::setprecision(10) << data.upperLimitsKD_perPos[pos1][maxmutIdx];
                    outfile << "\t" << std::setprecision(10) << utils::getPercentile(data.totalRelKD_perPos[pos1][maxmutIdx], 5);
                    outfile << "\t" << std::setprecision(10) << utils::getPercentile(data.totalRelKD_perPos[pos1][maxmutIdx], 95);
                    outfile << "\t" << std::setprecision(10) << utils::getPercentile(data.totalRelKD_perPos[pos1][maxmutIdx], 25);
                    outfile << "\t" << std::setprecision(10) << utils::getPercentile(data.totalRelKD_perPos[pos1][maxmutIdx], 75);
                    outfile << std::endl;
                }

            }
        } else {
            throw MIME_NoSuchFileException(KDFile.string());
        }
        outfile.close();
        return KDFile.string();
    }

    /*
	bool readPositionWiseKDEstimates(const string &resultDir, utils::DataContainer& data) {
        string filename = "PositionWiseKdEstimates.csv";
        fs::path KDFile(resultDir+"/KdResults/"+filename);
		
		bool successful = false;
		if(fs::exists(KDFile) && fs::is_regular_file(KDFile)) {
            fs::ifstream infile(KDFile.string());
            std::cout <<"Read file " << KDFile.string() << std::endl;
			if(infile.good()) {
				string line;
				//skip header line
				getline(infile, line);
				data.positions.clear();
				while(getline(infile, line)) {
					vector<string> splittedLine;
					boost::split(splittedLine, line, boost::is_any_of("\t"));
					int pos1 = std::stoi(splittedLine[0]);
					data.positions.push_back(pos1);
					data.maxMut[pos1] = std::stoi(splittedLine[2]);
					data.KDmedians[pos1].resize(3);
					data.pvalues[pos1].resize(3);
					data.numberOfKDs[pos1].resize(3);
					data.lowerLimitsKD_perPos[pos1].resize(3);
					data.upperLimitsKD_perPos[pos1].resize(3);
					for(int i= 0; i<3; ++i) {
                        data.KDmedians[pos1][i] = boost::lexical_cast<double>(splittedLine[i*7+3]);
                        data.pvalues[pos1][i] = boost::lexical_cast<double>(splittedLine[i*7+4]);
                        data.numberOfKDs[pos1][i] = boost::lexical_cast<double>(splittedLine[i*7+5]);
						data.lowerLimitsKD_perPos[pos1][i] = std::stoi(splittedLine[i*7+6]);
						data.upperLimitsKD_perPos[pos1][i] = std::stoi(splittedLine[i*7+7]);
// 						utils::getPercentile(data.totalRelKD_perPos[pos1][mut], 5);
// 						utils::getPercentile(data.totalRelKD_perPos[pos1][mut], 95);
					}
				}
				infile.close();
				successful = true;
			} else {
                throw MIME_NoSuchFileException(KDFile.string());
			}
		} else {
            throw MIME_NoSuchFileException(KDFile.string());
		}
		return successful;
	}
     */
    /*
    string writeParameterFile(const string& resultDir, utils::Parameter& param, utils::DataContainer& data) {
        fs::path confFile(resultDir + "/project.txt");
        fs::ofstream outfile(confFile.string());
        if(outfile.good()) {
            if(!param.refFile.empty())
                outfile << "refSeqFile\t" << param.refFile << std::endl;
            if(!param.dataDir.empty())
                outfile << "dataDir\t" << param.dataDir << std::endl;
//            outfile << "barcodeFile\t" << param.barcodeFile << std::endl;
            outfile << "alpha\t" << param.alpha << std::endl;
            outfile << "cutValueBwd\t" << param.cutValueBwd << std::endl;
            outfile << "cutValueFwd\t" << param.cutValueFwd << std::endl;
            outfile << "minimumNrCalls\t" << param.minimumNrCalls << std::endl;
            outfile << "minNumberEstimatableKds\t" << param.minNumberEstimatableKDs << std::endl;
            outfile << "minSignal2NoiseStrength\t" << param.minSignal2NoiseStrength<< std::endl;
            outfile << "minMutRate\t" << param.minMutRate << std::endl;
            outfile << "seqBegin\t" << param.seqBegin << std::endl;
            outfile << "seqEnd\t" << param.seqEnd << std::endl;
            outfile << "percOfMaxCov\t" << param.weightThreshold<< std::endl;
            outfile << "joinErrors\t" << std::boolalpha << param.joinErrors << std::endl;
            outfile << "plotYAxisFrom\t" << param.plotYAxisFrom<< std::endl;
            outfile << "plotYAxisTo\t" << param.plotYAxisTo<< std::endl;
            outfile << "plotStartRegion\t" << param.plotStartRegion<< std::endl;
            outfile << "plotEndRegion\t" << param.plotEndRegion<< std::endl;
            outfile << "signThreshold\t" << param.significanceThreshold << std::endl;


            for(auto s : data.bound) {
                outfile << "selected\t" << s.barcode << "\t" << s.name << "\t" << s.library << std::endl;
            }
            for(auto s : data.unbound) {
                outfile << "nonselected\t" << s.barcode << "\t" << s.name << "\t" << s.library << std::endl;
            }


//            outfile << "fullPlot\t" << std::boolalpha << param.fullPlot << std::endl;
//            outfile << "generateOutput\t" << std::boolalpha << param.generateOutput << std::endl;
//            outfile << "putMedian\t" << std::boolalpha << param.putMedian << std::endl;
//            outfile << "summaryPlot\t" << std::boolalpha << param.summaryPlot << std::endl;

//            outfile << "virion\t" << std::boolalpha << param.virion << std::endl;

        } else {
            throw MIME_NoSuchFileException(confFile.string());

        }
        outfile.close();
        return confFile.string();
    }
     */


    bool readParameterFile(const string& resultDir, utils::Parameter& param, utils::DataContainer& data) {
        fs::path confFile(resultDir);

        //if only directory is given, open existing parameter file, or open the given parameter file
        if(!fs::is_regular_file(confFile) && fs::is_directory(confFile)) {
            confFile += "/project.txt";
        }
        bool successful = false;
        //remove existing data
        data.bound.clear();
        data.unbound.clear();
        if(fs::exists(confFile) && fs::is_regular_file(confFile)) {
            std::ifstream infile(confFile.string());
            if(infile.good()) {
                string line;
                while(getline(infile, line)) {
                    vector<string> splittedLine = ioTools::split(line, '\t');
                    if(splittedLine[0] == "refSeqFile")
                        param.refFile = splittedLine[1];
                    else if(splittedLine[0] == "dataDir")
                        param.dataDir = splittedLine[1];
//                    else if(splittedLine[0] == "barcodeFile")
//                        param.barcodeFile = splittedLine[1];
                    else if(splittedLine[0] == "alpha")
                        param.alpha = std::stod(splittedLine[1]);
                    else if(splittedLine[0] == "cutValueBwd")
                        param.cutValueBwd = std::stoi(splittedLine[1]);
                    else if(splittedLine[0] == "cutValueFwd")
                        param.cutValueFwd = std::stoi(splittedLine[1]);
                    else if(splittedLine[0] == "minimumNrCalls")
                        param.minimumNrCalls = std::stoi(splittedLine[1]);
                    else if(splittedLine[0] == "minNumberEstimatableKds")
                        param.minNumberEstimatableKDs = std::stoi(splittedLine[1]);
                    else if(splittedLine[0] == "minSignal2NoiseStrength")
                        param.minSignal2NoiseStrength = std::stod(splittedLine[1]);
                    else if(splittedLine[0] == "minMutRate")
                        param.minMutRate = std::stod(splittedLine[1]);
                    else if(splittedLine[0] == "seqBegin")
                        param.seqBegin = std::stoi(splittedLine[1]);
                    else if(splittedLine[0] == "seqEnd")
                        param.seqEnd = std::stoi(splittedLine[1]);
                    else if(splittedLine[0] == "percOfMaxCov")
                        param.weightThreshold = std::stod(splittedLine[1]);
                    else if(splittedLine[0] == "joinErrors")
                         std::istringstream(splittedLine[1]) >> std::boolalpha >> param.joinErrors;
                    else if(splittedLine[0] == "plotYAxisFrom")
                        param.plotYAxisFrom = std::stod(splittedLine[1]);
                    else if(splittedLine[0] == "plotYAxisTo")
                        param.plotYAxisTo = std::stod(splittedLine[1]);
                    else if(splittedLine[0] == "plotStartRegion")
                        param.plotStartRegion = std::stoi(splittedLine[1]);
                    else if(splittedLine[0] == "plotEndRegion")
                        param.plotEndRegion = std::stoi(splittedLine[1]);
                    else if(splittedLine[0] == "selected")
                    {
                        utils::Sample sample(splittedLine[2], std::stoi(splittedLine[1]), utils::BOUND, std::stoi(splittedLine[3]));
                        data.bound.insert(sample);
                    } else if(splittedLine[0] == "nonselected")
                    {
                        utils::Sample sample(splittedLine[2], std::stoi(splittedLine[1]), utils::UNBOUND, std::stoi(splittedLine[3]));
                        data.unbound.insert(sample);
                    }else if(splittedLine[0] == "signThreshold") {
                        param.significanceThreshold = std::stod(splittedLine[1]);
                    }


//                    else if(splittedLine[0] == "fullPlot")
//                        std::istringstream(splittedLine[1]) >> std::boolalpha >> param.fullPlot;
                        //param.fullPlot = splittedLin[1];
//                    else if(splittedLine[0] == "generateOutput")
//                        std::istringstream(splittedLine[1]) >> std::boolalpha >> param.generateOutput;
                        //param.refFile = splittedLin[1];
//                    else if(splittedLine[0] == "putMedian")
//                        std::istringstream(splittedLine[1]) >> std::boolalpha >> param.putMedian;
                        //param.refFile = splittedLin[1];
//                    else if(splittedLine[0] == "summaryPlot")
//                        std::istringstream(splittedLine[1]) >> std::boolalpha >> param.summaryPlot;
                    else if(splittedLine[0] == "virion")
                        std::istringstream(splittedLine[1]) >> std::boolalpha >> param.virion;
                }
                infile.close();
                successful = true;

            } else {
                throw MIME_NoSuchFileException(confFile.string());

            }
        } else {
            throw MIME_NoSuchFileException(confFile.string());
        }

        return successful;
    }


     bool fileExists(const string& resultDir, const string& filename) {
        return ioTools::fileExists(resultDir + "/"+ filename);
     }

     bool fileExists(const string& filename) {
         fs::path file(filename);
         return (fs::exists(file) && fs::is_regular_file(file));
     }

     bool kDFileExists(const string& resultDir) {
         std::string kDDir(resultDir+"/KdResults");
        return (fileExists(kDDir, "rawKdValues.csv"));
     }

     bool errorFilesExist(const string& resultDir) {
          std::string errorDir(resultDir+"/errors");
         return (fileExists(errorDir, "errorEstimatesPerBaseSelected.csv") &&
                 fileExists(errorDir, "errorEstimatesPerBaseNonselected.csv"));
     }

     bool dirExists(const string& dirname) {
        fs::path dir(dirname);
        return (fs::exists(dir) && fs::is_directory(dir));
    }

     bool removeFile(const string& filename) {
          fs::path file(filename);
          return fs::remove(file);
     }

     bool removeErrorFiles(const string& resultDir) {
         fs::path errorDir(resultDir+"/errors");
         int numDel = fs::remove_all(errorDir);
         return(numDel>0);
     }

     bool removeKDFiles(const string& resDir) {
         fs::path kdDir(resDir+"/KdResults");
         int numDel = fs::remove_all(kdDir);
         return(numDel>0);
     }

     bool removeTmpFiles(const string& resDir) {
         fs::path kdDir(resDir+"/tmp");
         int numDel = fs::remove_all(kdDir);
         return(numDel>0);
     }
}
