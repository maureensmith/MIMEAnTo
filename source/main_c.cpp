//
// Created by Smith, Maureen on 05.08.18.
//
#include <iostream>
#include "utils.hpp"
#include "ioTools.hpp"
#include "plot.hpp"
#include "processing.hpp"
#include "mimeexception.hpp"

int main(int argc, char *argv[])
{

    // first check if enough parameter given
    if(argc < 2) {
        std::cerr << "Missing mandatory parameter for result directory (which contains the needed projext.txt\nMIMEAnTo resultdirectory <filename_suffix>" << std::endl;
        return 1;
    }


    utils::DataContainer data;
    utils::Parameter parameter;

    //std::string projectFile = argv[1];
    // load in the sample data
    if(ioTools::dirExists(argv[1])) {
        parameter.resultDir = argv[1];
    } else {

        std::string errorMsg = "Result directory does not exist:\n" + parameter.resultDir + "\n";
        //ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 1;
    }

    //remove tmp files from previous run
    if(ioTools::dirExists(parameter.resultDir))
        ioTools::removeTmpFiles(parameter.resultDir);

    // file name suffix to add to the saved plots and tables
    std::string fileName_suffix= "";
    if(argc == 3) {
        fileName_suffix = argv[2];
    }


    //read in the parameter file
    bool readSuccessful = ioTools::readParameterFile(parameter.resultDir, parameter, data);
    if(!readSuccessful)
    {
        std::string errorMsg = "Load data: \n  File does not exist:\n" + parameter.resultDir + "\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 1;
    }

    //ACHTUNG: former results are deleted
    if((ioTools::errorFilesExist(parameter.resultDir) || ioTools::kDFileExists(parameter.resultDir))) {

        ioTools::removeErrorFiles(parameter.resultDir);
        ioTools::removeKDFiles(parameter.resultDir);
    }

    //try to read reference file, if existing
    string refFileNative;
    if(ioTools::fileExists(parameter.refFile))
    {
        ioTools::readReference(parameter.refFile, data.ref);
    } else {
        std::string errorMsg = "Load data: \nFile does not exist:\n" + parameter.refFile + "\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 2;
    }

    if(parameter.seqEnd != 0 && parameter.seqBegin != 0) {
        parameter.cutValueFwd = parameter.seqBegin - 1;
        parameter.cutValueBwd =  data.ref.size() - parameter.seqEnd;
    }


    if((parameter.cutValueFwd + parameter.cutValueBwd) >= (int)data.ref.size() ||  parameter.seqBegin > parameter.seqEnd) {
        std::string errorMsg = "Load data: \nInvalid interval.\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 3;

    }

    //check if given data is balanced
    if(data.bound.size() != data.unbound.size() ) {
        std::string errorMsg = "Load data: \nNot enough data.\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 4;
    }

//    //TODO plot sanity check
//    //if wanted, plot sanity check
//    std::string mutRateSamplePlotFile;
//    bool plotSanity = false;
//    if(plotSanity) {
//        try {
//            plot::plotMutationRatePerSampleBoxplot(parameter.resultDir, parameter.dataDir, parameter, data, plot::PDF, fileName_suffix);
//
//        }
//        catch(MIME_GnuplotNotFoundException& e)
//        {
//            string errorMsg =  "Load data: \n" + e.message();
//            ioTools::writeErrorLog(parameter.resultDir, errorMsg);
//            std::cerr << errorMsg << std::endl;
//            return 5;
//        }
//        catch(MIME_PathToExecutableNotFoundException& e)
//        {
//            string errorMsg =  "Load data: \n" + e.message();
//            ioTools::writeErrorLog(parameter.resultDir, errorMsg);
//            std::cerr << errorMsg << std::endl;
//            return 5;
//        }
//        catch(MIME_NoSuchFileException& e)
//        {
//            std::string errorMsg = "Load data: \nFile does not exist: " + e.getFilename() + "\n";
//            ioTools::writeErrorLog(parameter.resultDir, errorMsg);
//            std::cerr << errorMsg << std::endl;
//            return 5;
//        }
//        catch(exception& e)
//        {
//            string errorMsg =  "Load data: \n" + std::string(e.what());
//            ioTools::writeErrorLog(parameter.resultDir, errorMsg);
//            std::cerr << errorMsg << std::endl;
//            return 5;
//        }
//
//    }

    /***** error estimation ****/

    try {
        processing::estimateError(parameter.dataDir, parameter, data);
    }
    catch(MIME_NoSuchFileException& e) {
        std::string errorMsg = "Estimate error: \n File does not exist:\n" +e.getFilename() + "\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 6;
    }
    catch(exception& e)
    {
        std::string errorMsg = "Estimate error: \n"+  std::string(e.what()) + "\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 6;
    }


    //TODO errors rausschreiben?
    ioTools::writeErrorEstimates(parameter.resultDir, data);

    //TODO plot Error

    /***** compute raw KD values ****/

    try
    {
        processing::computeKDvalues(parameter.dataDir, parameter, data);
        //TODO if it's only about the plots, rawKds are not necessary (take a lot of space)
        //save raw Kds automatically
        //ioTools::writeRawKDCriteria(parameter.resultDir, data, "rawKdValues.csv");
    }
    catch(exception& e)
    {
        std::string errorMsg = "Compute raw KDs: \nAn error occured during the computation of the raw Kd values.\n" + std::string(e.what()) + "\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 7;
    }


    /***** apply quality criteria ****/

    try {
        processing::applyQualityCriteria(parameter, data);
    } catch(exception& e)
    {
        std::string errorMsg = "Apply quality criteria: \nAn error occured during the application of the quality criteria.\n" + std::string(e.what()) + "\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 8;
    }


    /*** plot Kds ***/
    //TODO at moment not possible. Extra gnuplot call with the gp files
    try {
        plot::plotAllEffects(parameter.resultDir, parameter, data, plot::PDF, fileName_suffix);
    }catch(MIME_GnuplotNotFoundException& e)
    {
        std::string errorMsg ="Save Kd plot: \n" + e.message();
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 8;
    }
    catch(MIME_PathToExecutableNotFoundException& e)
    {
        std::string errorMsg ="Save Kd plot: \n" + e.message();
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 8;
    }
    catch(std::exception& e)
    {
        std::string errorMsg = "Save Kd plot: \nSomething went wrong with plot. The error is logged in the file tmp/gnuplotError.log\n" + std::string(e.what()) + "\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 8;
    }

    // save Kds
    std::string filename;
    try
    {
        ioTools::writePositionWiseKDEstimates(parameter.resultDir, data, fileName_suffix);
        ioTools::writePositionWiseMaxKD(parameter.resultDir, data, fileName_suffix);
    }
    catch(std::exception& e)
    {
        std::string errorMsg = "Save Kd results: \nSomething went wrong with saving.\n" + std::string(e.what()) + "\n";
        ioTools::writeErrorLog(parameter.resultDir, errorMsg);
        std::cerr << errorMsg << std::endl;
        return 9;
    }


    return 0;
}