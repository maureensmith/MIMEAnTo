#pragma once
#include <string>

#include "utils.hpp"


using namespace std;

namespace ioTools {

    std::vector<std::string> split(const std::string& s, char delimiter);

    std::string createDir(const string& resultDir);

    void writeErrorLog(const string& resultDir, const std::string& text);
	
    string readReference(const string& refStr, utils::refMap& ref);

    void readExperimentFile(int barcode, const string& expDir, utils::countsPerPosPair& countsPP);
	
    void readExperimentFile(int barcode, const string& expDir, int pos, utils::countsPerSecondPos& countsPP);

    void readExperimentFileAndSum(int barcode, const string& expDir, std::map<int, utils::rateArray>& counts);
    
    void readAndCompute1dMutationRate(int barcode, const string& expDir, utils::DataContainer& data, std::vector<double>& rates);

    void writeErrorEstimates(const string& resultDir, utils::DataContainer& data);
    
    void readErrorEstimates(const string& resultDir, utils::DataContainer& data);
	
    void writeRawKDValues(const string& resultDir, utils::DataContainer& data, const string& filename);
    
    void readRawKDValues(const string& resultDir, utils::DataContainer& data, const string& filename);

    void writeSignal2Noise(const string& resultDir, utils::DataContainer& data);

    void readSignal2Noise(const string& resultDir, utils::DataContainer& data);

    void writePositionWeights(const string& resultDir, utils::DataContainer& data);

    void readPositionWeights(const string& resultDir, utils::DataContainer& data);

    void writeSequenceNumbers(const string& resultDir, utils::DataContainer& data);

    void readSequenceNumbers(const string& resultDir, utils::DataContainer& data);

    void readRawKDCriteria(const string& resultDir, utils::DataContainer& data, const string& filename);

    void writeRawKDCriteria(const string& resultDir, utils::DataContainer& data, const string &filename);
	
    std::string writePositionWiseKDEstimates(const string& resultDir, utils::DataContainer& data, const string& filenamePrefix = "");

    std::string writePositionWiseMaxKD(const string &resDir, utils::DataContainer& data, const string &filenamePrefix = "");
    
    bool readPositionWiseKDEstimates(const string& resultDir, utils::DataContainer& data);

    string writeParameterFile(const string& resultDir, utils::Parameter& param, utils::DataContainer &data);

    bool readParameterFile(const string& resultDir, utils::Parameter& param, utils::DataContainer &data);

    bool fileExists(const string& resultDir, const string& filename);

    bool fileExists(const string& filename);

    bool dirExists(const string& dirname);

    bool kDFileExists(const string& resultDir);

    bool errorFilesExist(const string& resultDir);

    bool removeErrorFiles(const string& resultDir);

    bool removeKDFiles(const string& resultDir);

    bool removeTmpFiles(const string& resultDir);
}
