#include "plot.hpp"
#include <fstream>
#include <cstdlib>
#include "ioTools.hpp"
#include "mimeexception.hpp"
#include "gnuplot-iostream/gnuplot-iostream.h"

//For finding the path to the executable on each platform
#ifdef __APPLE__
//For TARGET_OS_MAC
#include "TargetConditionals.h"
#include <mach-o/dyld.h>
#elif defined _WIN32 || defined _WIN64 || defined WIN32 || defined WIN64
#include "windows.h"
#elif defined __linux__
#include <unistd.h>
#endif

namespace fs = boost::filesystem;

namespace plot {



    //Call gnuplot executable with gp file in Linux, Windows and OSX
    void callGnuplot(fs::path& gpFile, const string& resDir)
    {

        fs::path gnuplotLogFile(resDir+"/tmp/gnuplotError.log");
        std::string gnuplotExe = "";
        // last <\svg> not added if piped to exe. So: save as gp file and call exe with gp file
#if defined _WIN32 || defined _WIN64 || defined WIN32 || defined WIN64
        TCHAR execPath[MAX_PATH];

        if(GetModuleFileName(NULL, execPath, MAX_PATH))
        {
            fs::path execFile(execPath);
            gnuplotExe = "\"" + execFile.parent_path().string() + "\\gnuplot\\bin\\gnuplot.exe " + gpFile.string() + " 2>> " + gnuplotLogFile.string() + "\"";
        }
        else
            throw MIME_PathToExecutableNotFoundException();
#elif defined __APPLE__
    #ifdef TARGET_OS_MAC
        //find path to executable to create relative path to gnuplot
        char execPathBuf[PATH_MAX];
        uint32_t size = PATH_MAX;
        if(!_NSGetExecutablePath(execPathBuf, &size))
        {
            fs::path execFile(execPathBuf);
            // if directory with gnuplot is present in the same directory as executable (Console version), othrewise take the packed version of dmg
            if(ioTools::fileExists(execFile.parent_path().string() +"/gnuplot/Gnuplot.app/Contents/Resources/bin/gnuplot-run.sh")) {
                gnuplotExe = execFile.parent_path().string() +"/gnuplot/Gnuplot.app/Contents/Resources/bin/gnuplot-run.sh " + gpFile.string() + " 2>> " + gnuplotLogFile.string();
            } else {
                //Find path to executable and add the relative path to gnuplot, add the gp file to the command and pipe the error to the logFile
                gnuplotExe = execFile.parent_path().string() +"/../PlugIns/Gnuplot.app/Contents/Resources/bin/gnuplot-run.sh "+ gpFile.string() + " 2>> " + gnuplotLogFile.string();
            }
        } else
            throw MIME_PathToExecutableNotFoundException();
    #endif
#elif defined __linux__
        //test if gnuplot is installed (by aksing for the help), if not use executable
        std::string gnuplotCall = "gnuplot -h >/dev/null";
       if(std::system(gnuplotCall.c_str()) != 0)
       {
//           char execPathBuf[PATH_MAX];
//           if (readlink("/proc/self/exe", execPathBuf, PATH_MAX) != -1)
//           {
//               fs::path execFile(execPathBuf);
//               gnuplotExe = execFile.parent_path().string() + "/gnuplot/bin/gnuplot " + gpFile.string() + " 2>> " + gnuplotLogFile.string();
//           }
//           else
           //Gnuplot is not installed -> Exception (it needs to be installed on linux)
             throw MIME_GnuplotNotFoundException();
       } else
       {
           gnuplotExe = "gnuplot " + gpFile.string() + " 2>> " + gnuplotLogFile.string();
       }
#endif

       //call gnuplot
        if(gnuplotExe.size() > 0)
            std::system(gnuplotExe.c_str());
    }



    string plotBoxplot(const string& resDir, const string& expDir, utils::Parameter& param, utils::DataContainer& data, bool svg, const string &filenamePrefix, bool eps) {
		std::cout << "Plot mutation rates per sample... " << std::endl;
		//determine the mutation ratefor each sample for a first "sanity check"
		fs::path *file;
		Gnuplot *gp;
        ioTools::createDir(resDir+"/tmp");
        ioTools::createDir(resDir+"/plots");

        fs::path gpFile(resDir+"/tmp/mutRateBoxPlot.gp");
        //call is saved in gp-file and called afterwards
        std::string gnuplotCall = ">"+gpFile.string();

		if(svg) 
		{
            file = new fs::path(resDir+"/tmp/mutRateBoxPlot.svg");
            gp = new Gnuplot(gnuplotCall);

			*gp << "set term svg enhanced\n";

        } else if(eps) {
            std::string fileName = "/mutRateBoxPlot";
            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".eps");
            gp = new Gnuplot(gnuplotCall);
            *gp << "set term postscript eps enhanced color solid\n";
            *gp << "set title 'Mutation frequency per sample'\n";
        } else
		{
            std::string fileName = "/mutRateBoxPlot";
            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".pdf");
            gp = new Gnuplot(gnuplotCall);
            *gp << "set term pdfcairo enhanced color font 'Verdana, 8'\n";
            *gp << "set title 'Mutation frequency per sample'\n";
		}

        *gp << "set bmargin 5\n";
        *gp << "set lmargin 10\n";

        *gp << "set output '" << file->string() << "'\n";
		*gp << "set style fill solid 0.25 border -1\n";
		*gp << "set style boxplot outliers pointtype 7\n";
		*gp << "set style data boxplot\n";
		*gp << "set boxwidth  0.5\n";
		*gp << "set pointsize 0.3\n";
		*gp << "set grid ytics linestyle 0\n";
		*gp << "set xtics nomirror\n";
		*gp << "set ytics nomirror\n";
		*gp << "set key out vert\n";
		*gp << "set key top right\n";
		*gp << "set xlabel 'Sample'\n";
        *gp << "set ylabel 'Mismatch frequency (log_{10})'\n";
		utils::sampleContainer::iterator boundIt, unboundIt, dnaIt;

        //workaround: print the first sample as key

		bool wt = false;
		bool mut = false;
        bool first = true;
		for(boundIt=data.bound.begin(); boundIt != data.bound.end(); ++boundIt) {
            if(!boundIt->active)
                continue;
            int barcode = (*boundIt).barcode;
            int lib = (*boundIt).library;
            string color = "cyan";
            string title = "title 'selected'";
            if(lib == 0) {
                color = "dark-blue"; //darkblue
                title = "title 'selected wt'";
                if(wt)
                    title = "notitle";
                wt = true;
            } else {
                if(mut)
                    title = "notitle";
                mut = true;
            }

            if(first)
            {
                *gp << "plot ";
                first = false;
           } else
                *gp << ", ";
            *gp << "'-' using ("<<barcode <<"):1 " << title <<" lc rgb '"<< color << "'";
		}


		wt = false;
		mut = false;
		for(unboundIt=data.unbound.begin(); unboundIt != data.unbound.end(); ++unboundIt) {
            if(!unboundIt->active)
                continue;
            int barcode = (*unboundIt).barcode;
            int lib = (*unboundIt).library;
            string color = "red";
            string title = "title 'non-selected'";
            if(lib == 0) {
                color = "dark-red"; //darkred
                title = "title 'non-selected wt'";
                if(wt)
                    title = "notitle";
                wt = true;
            } else {
                if(mut)
                    title = "notitle";
                mut = true;
            }
            *gp << ", '-' using ("<<barcode <<"):1 " << title <<" lc rgb '"<< color << "'";
         }


		wt = false;
		mut = false;
		for(dnaIt=data.dna.begin(); dnaIt != data.dna.end(); ++dnaIt) {
            if(!dnaIt->active)
                continue;
            int barcode = (*dnaIt).barcode;
            int lib = (*dnaIt).library;
            string color = "yellow";
            string title = "title 'dna'";
            if(lib == 0) {
                color = "orange";
                title = "title 'dna wt'";
                if(wt)
                    title = "notitle";
                wt = true;
            } else {
                if(mut)
                    title = "notitle";
                mut = true;
            }
            *gp << ", '-' using ("<<barcode <<"):1 " << title <<" lc rgb '"<< color << "'";

		}
		*gp << "\n";
		
		for(boundIt=data.bound.begin(); boundIt != data.bound.end(); ++boundIt) {
            if(!boundIt->active)
                continue;
            int barcode = (*boundIt).barcode;
            std::vector<double> rates;
            ioTools::readAndCompute1dMutationRate(barcode, expDir+"/1d", data, rates);
            std::vector<double> logRates(param.seqEnd - param.seqBegin);
            std::transform(rates.begin()+param.seqBegin-1, rates.begin()+param.seqEnd-1, logRates.begin(), (double(*)(double))log10);
            gp->send1d(logRates);


		}

		
		for(unboundIt=data.unbound.begin(); unboundIt != data.unbound.end(); ++unboundIt) {
            if(!unboundIt->active)
                continue;
			int barcode = (*unboundIt).barcode;
// 			int lib = (*unboundIt).library;
			std::vector<double> rates;
			ioTools::readAndCompute1dMutationRate(barcode, expDir+"/1d", data, rates);
			std::vector<double> logRates(param.seqEnd - param.seqBegin);
			std::transform(rates.begin()+param.seqBegin-1, rates.begin()+param.seqEnd-1, logRates.begin(), (double(*)(double))log10);
            gp->send1d(logRates);
		}
		
		
		for(dnaIt=data.dna.begin(); dnaIt != data.dna.end(); ++dnaIt) {
            if(!dnaIt->active)
                continue;
			int barcode = (*dnaIt).barcode;
// 			int lib = (*dnaIt).library;
			std::vector<double> rates;
			ioTools::readAndCompute1dMutationRate(barcode, expDir+"/1d", data, rates);
			std::vector<double> logRates(param.seqEnd - param.seqBegin);
			std::transform(rates.begin()+param.seqBegin-1, rates.begin()+param.seqEnd-1, logRates.begin(), (double(*)(double))log10);
			gp->send1d(logRates);
		}

        delete gp;
        callGnuplot(gpFile, resDir);
        std::string fileStr = file->string();
        delete file;
        return fileStr;
	}

    string plotMutationRatePerSampleBoxplot(const string& resDir, const string& expDir, utils::Parameter& param, utils::DataContainer& data, plot::PlotFormat plotformat, const string &filenamePrefix) {
        bool svg = plotformat == plot::SVG;
        bool eps = plotformat == plot::EPS;
        string file = plotBoxplot(resDir, expDir, param, data, svg, filenamePrefix, eps);
        return file;
    }


    string plotCoeffVariation(const string& resDir, utils::Parameter& param, utils::DataContainer& data, bool svg, const string& filenamePrefix, bool eps)
    {
        std::cout << "Plot coefficient of variation of median error rate per sample... " << std::endl;
        std::map<int, utils::rateArray> meanErrorsPerPosBound;
        std::map<int, double> varCoeffPerPosBound;
        std::map<int, utils::rateArray> meanErrorsPerPosUnbound;
        std::map<int, double> varCoeffPerPosUnbound;

        data.maxCoVarErrorBound = 0.0;
        data.maxCoVarErrorUnbound = 0.0;
        double maxCoeffBound = 0.0;
        double maxCoeffUnbound = 0.0;

        ioTools::createDir(resDir+"/tmp");
        ioTools::createDir(resDir+"/plots");

        //count activated samples
        int i = 0;
        for(auto sampleBoundIt = data.bound.begin(), sampleUnboundIt = data.unbound.begin(); sampleBoundIt != data.bound.end(); ++sampleBoundIt, ++sampleUnboundIt)
        {
            if(sampleBoundIt->library == 0 && sampleBoundIt->active)
                ++i;
        }

        int j = 0;
        for(auto sampleBoundIt = data.bound.begin(), sampleUnboundIt = data.unbound.begin(); sampleBoundIt != data.bound.end(); ++sampleBoundIt, ++sampleUnboundIt)
        {
            if(sampleBoundIt->library == 0 && sampleBoundIt->active)
            {
                int boundBarcode = sampleBoundIt->barcode;
                int unboundBarcode = sampleUnboundIt->barcode;

                for(auto posItBound = data.medianExpKappaTotal_perSampleBound[boundBarcode].begin(); posItBound != data.medianExpKappaTotal_perSampleBound[boundBarcode].end(); ++posItBound)
                {
                    int pos = posItBound->first;

                    /*coefficient of variation*/
                    if(meanErrorsPerPosBound[pos].size() == 0)
                    {
                        meanErrorsPerPosBound[pos].resize(i);
                        meanErrorsPerPosUnbound[pos].resize(i);
                    }
                    meanErrorsPerPosBound[pos][j] = data.medianExpKappaTotal_perSampleBound[boundBarcode][pos];
                    meanErrorsPerPosUnbound[pos][j] = data.medianExpKappaTotal_perSampleUnbound[unboundBarcode][pos];



                    //if last element is added, compute coefficient of variation
                    if(i == j+1)
                    {

                        varCoeffPerPosBound[pos] = utils::getCoeffVar(meanErrorsPerPosBound[pos])*100;
                        varCoeffPerPosUnbound[pos] = utils::getCoeffVar(meanErrorsPerPosUnbound[pos])*100;

                        if(varCoeffPerPosBound[pos] > maxCoeffBound)
                            maxCoeffBound = varCoeffPerPosBound[pos];
                        if(varCoeffPerPosUnbound[pos] > maxCoeffUnbound)
                            maxCoeffUnbound = varCoeffPerPosUnbound[pos];
                    }
                }
                ++j;
            }
        }

        Gnuplot* gp;
        fs::path* file;
        fs::path gpFile(resDir+"/tmp/coefficientOfVariation.gp");
        std::string gnuplotCall = ">"+gpFile.string();

        //Coefficient of variation: standard deviation/mean
        if(svg)
        {
            file = new fs::path(resDir+"/tmp/coefficientOfVariation.svg");
            gp = new Gnuplot(gnuplotCall);

            *gp << "set output '" << (*file).string() << "'\n";
            *gp << "set term svg enhanced\n";
        }
        else if(eps)
        {
            std::string fileName = "/coefficientOfVariation";
            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".eps");
            gp = new Gnuplot(gnuplotCall);
            *gp << "set output '" << (*file).string() << "'\n";
            *gp << "set term postscript eps enhanced color\n";
            *gp << "set title 'Coefficient of variation of mean error estimates'\n";
        }
        else
        {
            std::string fileName = "/coefficientOfVariation";
            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".pdf");
            gp = new Gnuplot(gnuplotCall);
            *gp << "set output '" << (*file).string() << "'\n";
            *gp << "set term pdfcairo enhanced color font 'Verdana, 8'\n";
            *gp << "set title 'Coefficient of variation of mean error estimates'\n";
        }

        *gp << "set bmargin 5\n";
        *gp << "set lmargin 10\n";

        *gp << "set grid ytics linestyle 0\n";
        *gp << "set xrange [" << param.seqBegin-5 << ":" << param.seqEnd+5 << "]\n";
        if(maxCoeffBound < 10.0 && maxCoeffUnbound < 10.0)
            *gp << "set yrange [-0.5:10]\n";
        else
             *gp << "set yrange [-0.5:" << max(maxCoeffBound, maxCoeffUnbound)+1.0 << "]\n";

        *gp << "set xlabel 'Sequence position'\n";
        *gp << "set ylabel 'Coefficient of variation (%)'\n";
        *gp << "set key outside right vertical top Right\n";

        *gp << "plot '-' with line title 'selected' lw 2 lt -1 lc rgb '#56b4e9', '-' with line title 'non-selected' lw 2 lt -1 lc rgb '#e51e10'\n";
        (*gp).send1d(varCoeffPerPosBound);
        (*gp).send1d(varCoeffPerPosUnbound);

        delete gp;
        callGnuplot(gpFile, resDir);
        std::string fileStr = file->string();
        delete file;
        return fileStr;
    }


    string plotCoefficientOfVariation(const string& resDir, utils::Parameter& param, utils::DataContainer& data, plot::PlotFormat plotformat, const string& filenamePrefix) {

        bool svg = plotformat == plot::SVG;
        bool eps = plotformat == plot::EPS;
        string file = plotCoeffVariation(resDir, param, data, svg, filenamePrefix, eps);
        return file;
    }


	
    string plotError(const string& resDir, utils::Parameter& param, utils::DataContainer& data, bool svg, const string &filenamePrefix, bool eps) {
		std::cout << "Plot median error rate per sample... " << std::endl;

        int linewidth = 2;
        ioTools::createDir(resDir+"/tmp");
        ioTools::createDir(resDir+"/plots");
        Gnuplot* gp;
        fs::path* file;

        fs::path gpFile(resDir+"/tmp/errorEstimates.gp");
        std::string gnuplotCall = ">"+gpFile.string();
        if(svg) {
            file = new fs::path(resDir+"/tmp/errorEstimates.svg");
            gp = new Gnuplot(gnuplotCall);

            *gp << "set output '" << (*file).string() << "'\n";
            *gp << "set term svg enhanced\n";
            *gp << "set multiplot layout 2,1\n";
        } else if(eps) {
            linewidth = 4;
            std::string fileName = "/errorEstimates";

            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".eps");
            gp = new Gnuplot(gnuplotCall);

            *gp << "set output '" << (*file).string() << "'\n";
            //pdf instead of eps, because transparency doesn't work
            *gp << "set term postscript eps enhanced color\n";
            //*gp << "set term pdfcairo enhanced color font 'Verdana, 8'\n";
            *gp << "set multiplot layout 2,1 title 'Error estimates per sample'\n";

        }
        else
        {
            linewidth = 4;
            std::string fileName = "/errorEstimates";

            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".pdf");
            gp = new Gnuplot(gnuplotCall);

            *gp << "set output '" << (*file).string() << "'\n";
            *gp << "set term pdfcairo enhanced color font 'Verdana, 8'\n";
            *gp << "set multiplot layout 2,1 title 'Error estimates per sample'\n";

        }

        //set colors (because they are different for gnuplotversion 4.6 and 5.0
        *gp << "set linetype  1 lc rgb 'dark-violet' lw 1\n";
        *gp << "set linetype  2 lc rgb '#009e73' lw 1\n";
        *gp << "set linetype  3 lc rgb '#56b4e9' lw 1\n";
        *gp << "set linetype  4 lc rgb '#e69f00' lw 1\n";
        *gp << "set linetype  5 lc rgb '#f0e442' lw 1\n";
        *gp << "set linetype  6 lc rgb '#0072b2' lw 1\n";
        *gp << "set linetype  7 lc rgb '#e51e10' lw 1\n";
        *gp << "set linetype  8 lc rgb 'black' lw 1\n";
        *gp << "set linetype  9 lc rgb 'gray50' lw 1\n";

         /****** bound errors******/

        //set margrins for multiplot: no gap between plot, enough space for axis labels
        *gp << "set tmargin 1\n";
//             *gp << "set bmargin 0\n";
        *gp << "set lmargin 10\n";
        *gp << "set rmargin 20\n";
        *gp << "set style fill transparent solid 0.25 noborder\n";
        *gp << "set format x ''\n";
        *gp << "set xrange [" << param.seqBegin-5 << ":" << param.seqEnd+5 << "]\n";
        *gp << "set format y\n";

        *gp << "set ylabel 'median error freq.(log_{10})'\n";
        *gp << "set grid ytics linestyle 0\n";
        *gp << "set key outside right vertical top Right title 'selected'\n";
        //first the percentiles so they are in the background
        int i = 1;
        for(auto sampleBoundIt = data.bound.begin(); sampleBoundIt != data.bound.end(); ++sampleBoundIt)
        {
            if(sampleBoundIt->library == 0 && sampleBoundIt->active)
            {
                // for the first run add the plot, and initialize the coefficient of variation arrays
                if(i==1)
                {
                    /*error rate*/
                    *gp << "plot ";
                } else
                {
                    /*error rate*/
                    *gp << ", ";
                }
                //std as background
                *gp << "'-' notitle w filledcu lc "<< i;

                ++i;
            }
        }

        //then the median
        i = 1;
        for(auto sampleBoundIt = data.bound.begin(); sampleBoundIt != data.bound.end(); ++sampleBoundIt)
        {
            if(sampleBoundIt->library == 0 && sampleBoundIt->active)
            {
                std::string name = sampleBoundIt->name;
                //median error as line
                *gp << ",'-' using 1:2 with line title '" << name <<"' lw "<< linewidth << " lt -1 lc "<< i;
                ++i;
            }
        }

        *gp << "\n";

        //first put the percentile data
        for(auto sampleBoundIt = data.bound.begin(); sampleBoundIt != data.bound.end(); ++sampleBoundIt)
        {
            if(sampleBoundIt->library == 0 && sampleBoundIt->active)
            {
                int boundBarcode = sampleBoundIt->barcode;
                std::map<int, std::pair<double, double>> logPercAreaBound;

                for(auto posItBound = data.medianExpKappaTotal_perSampleBound[boundBarcode].begin(); posItBound != data.medianExpKappaTotal_perSampleBound[boundBarcode].end(); ++posItBound)
                {
                    int pos = posItBound->first;
                    /*error rate*/
                    logPercAreaBound[pos] = std::make_pair(log10(data.perc75ExpKappaTotal_perSampleBound[boundBarcode][pos]), log10(data.perc25ExpKappaTotal_perSampleBound[boundBarcode][pos]));
                }
                /*error rate*/
                (*gp).send1d(logPercAreaBound);
            }
		}

        //then the median data
        for(auto sampleBoundIt = data.bound.begin(); sampleBoundIt != data.bound.end(); ++sampleBoundIt)
        {
            if(sampleBoundIt->library == 0 && sampleBoundIt->active)
            {
                int boundBarcode = sampleBoundIt->barcode;
                utils::ratePerPos logMedianErrorsPerPosBound;

                for(auto posItBound = data.medianExpKappaTotal_perSampleBound[boundBarcode].begin(); posItBound != data.medianExpKappaTotal_perSampleBound[boundBarcode].end(); ++posItBound)
                {
                    int pos = posItBound->first;
                    /*error rate*/
                    logMedianErrorsPerPosBound[pos] = log10(data.medianExpKappaTotal_perSampleBound[boundBarcode][pos]);
                }

                (*gp).send1d(logMedianErrorsPerPosBound);
            }
        }

        /****** unbound errors******/

        *gp << "set format x\n";
        *gp << "set ylabel 'median error freq.(log_{10})'\n";
        *gp << "set tmargin 0\n";
        *gp << "set bmargin 5\n";
        *gp << "set xlabel 'Sequence position'\n";
        *gp << "set key title 'non-selected'\n";

        i = 1;
        for(auto sampleUnboundIt = data.unbound.begin(); sampleUnboundIt != data.unbound.end(); ++sampleUnboundIt)
        {
            if(sampleUnboundIt->library == 0 && sampleUnboundIt->active)
            {
                // for the first run add the plot, and initialize the coefficient of variation arrays
                if(i==1)
                {
                    /*error rate*/
                    *gp << "plot ";
                } else
                {
                    /*error rate*/
                    *gp << ", ";
                }
                //std as background
                *gp << "'-' notitle w filledcu lc "<< i;
                ++i;
            }
        }

        i = 1;
        for(auto sampleUnboundIt = data.unbound.begin(); sampleUnboundIt != data.unbound.end(); ++sampleUnboundIt)
        {
            if(sampleUnboundIt->library == 0 && sampleUnboundIt->active)
            {
                std::string name = sampleUnboundIt->name;

                //median error as line
                *gp << ", '-' using 1:2 with line title '" << name <<"' lw "<< linewidth << " lt 0 lc "<< i;
                ++i;
            }
        }

        *gp << "\n";
        for(auto sampleUnboundIt = data.unbound.begin(); sampleUnboundIt != data.unbound.end(); ++sampleUnboundIt)
        {
            if(sampleUnboundIt->library == 0 && sampleUnboundIt->active)
            {
                int unboundBarcode = sampleUnboundIt->barcode;
                 std::map<int, std::pair<double, double>> logPercAreaUnbound;

                for(auto posItUnbound = data.medianExpKappaTotal_perSampleUnbound[unboundBarcode].begin(); posItUnbound != data.medianExpKappaTotal_perSampleUnbound[unboundBarcode].end(); ++posItUnbound)
                {
                    int pos = posItUnbound->first;
                    logPercAreaUnbound[pos] = std::make_pair(log10(data.perc75ExpKappaTotal_perSampleUnbound[unboundBarcode][pos]), log10(data.perc25ExpKappaTotal_perSampleUnbound[unboundBarcode][pos]));
                }
                (*gp).send1d(logPercAreaUnbound);
            }
        }

        for(auto sampleUnboundIt = data.unbound.begin(); sampleUnboundIt != data.unbound.end(); ++sampleUnboundIt)
        {
            if(sampleUnboundIt->library == 0 && sampleUnboundIt->active)
            {
                int unboundBarcode = sampleUnboundIt->barcode;
                utils::ratePerPos logMedianErrorsPerPosUnbound;
                for(auto posItUnbound = data.medianExpKappaTotal_perSampleUnbound[unboundBarcode].begin(); posItUnbound != data.medianExpKappaTotal_perSampleUnbound[unboundBarcode].end(); ++posItUnbound)
                {
                    int pos = posItUnbound->first;
                    logMedianErrorsPerPosUnbound[pos] = log10(data.medianExpKappaTotal_perSampleUnbound[unboundBarcode][pos]);

                }
                (*gp).send1d(logMedianErrorsPerPosUnbound);
            }
        }

        *gp << "unset multiplot\n";

        delete gp;
        callGnuplot(gpFile, resDir);
        std::string fileStr = file->string();
        delete file;
        return fileStr;
	}

    string plotMedianErrorPerSample(const string& resDir, utils::Parameter& param, utils::DataContainer& data, plot::PlotFormat plotformat, const string &filenamePrefix) {

        bool eps = plotformat == plot::EPS;
        bool svg = plotformat == plot::SVG;
        string file = plotError(resDir, param, data, svg, filenamePrefix, eps);
        return file;
    }


    string plotAllKd(const string& resDir, utils::Parameter& param, utils::DataContainer& data, bool svg, const string &filenamePrefix, bool eps) {
		std::cout << "Plot KD values with all effects ..." << std::endl;

		/****Data for the KD boxplots for all mutations ****/
        std::vector<std::string> ACGT{"A", "C", "G", "U"};
		
		std::vector<std::vector<double>> medians(3);
		std::vector<std::vector<double>> perc95(3);
		std::vector<std::vector<double>> perc5(3);
		std::vector<std::vector<double>> perc75(3);
		std::vector<std::vector<double>> perc25(3);

        std::vector<std::vector<double>> minIQR(3);
        std::vector<std::vector<double>> maxIQR(3);
		
		std::vector<std::vector<int>> colors(3);
		
		//color of the stars (if significant) per mutation for each position together with the color (white if small p-value -> no star)
		//workaround: alle einzeln, da nicht möglich mehrere Farben pro label zu setzen
		std::vector<std::vector<std::pair<int, int>>> posEffPValue(3);
		std::vector<std::vector<std::pair<int, int>>> negEffPValue(3);
		
		std::vector<boost::tuple<int, std::string, int>> refStr;
		
        ioTools::createDir(resDir+"/tmp");
        ioTools::createDir(resDir+"/plots");
		
		/**** data for the max effect plot ***/
		std::vector<std::pair<int,double>> a2g;
		std::vector<double> mediansMax;
		std::vector<double> perc95Max;
		std::vector<double> perc5Max;
		std::vector<double> perc75Max;
		std::vector<double> perc25Max;
		std::vector<int> actPositionsMax;
        std::vector<int> actPositions;
        std::vector<boost::tuple<int,double, int>> pValuesMaxEff;
		
		int minY = 0;
		int maxY = 0;
		int minYMaxEff = 0;
		int maxYMaxEff = 0;

        int begin = max(param.plotStartRegion, param.seqBegin);
        int end = min(param.plotEndRegion, param.seqEnd);
        double yFrom = param.plotYAxisFrom;
        double yTo = param.plotYAxisTo;

        double yRef;

        int font = 2;

        if(svg) {
            font = 10;
            begin = param.seqBegin;
            end = param.seqEnd;
        }
		
        std::vector<std::pair<double, int>> nullLine{std::make_pair(begin-0.5, 0), std::make_pair(end+0.5, 0)};

        for(int pos = begin; pos <=end; ++pos){
            std::map<int,int>::iterator maxIt = data.maxMut.find(pos);
            int maxmut = -1;
            int maxmutIdx = -1;
            if(maxIt != data.maxMut.end() && !std::isnan(maxIt->second) && (maxIt->second) > 0)
            {
				/*** max effect computation ***/
                //int maxmut = data.maxMut[pos];
                maxmut = maxIt->second;
                maxmutIdx = maxmut-1;
                //if(!std::isnan(maxmut) && maxmut > 0) {

                if(maxmut > data.ref[pos])
                    --maxmutIdx;
                //determine number/index of mutation with maximum effect
                double p95 = log2(utils::getPercentile(data.totalRelKD_perPos[pos][maxmutIdx], 95));
                double p5 = log2(utils::getPercentile(data.totalRelKD_perPos[pos][maxmutIdx], 5));
                mediansMax.push_back(log2(data.KDmedians[pos][maxmutIdx]));
                perc95Max.push_back(p95);
                perc5Max.push_back(p5);
                perc75Max.push_back(log2(utils::getPercentile(data.totalRelKD_perPos[pos][maxmutIdx], 75)));
                perc25Max.push_back(log2(utils::getPercentile(data.totalRelKD_perPos[pos][maxmutIdx], 25)));
                actPositionsMax.push_back(pos);

// 				}
                if(data.ref[pos] == 1 && maxmut == 3)
                    a2g.push_back(std::make_pair(pos, log2(data.KDmedians[pos][maxmutIdx])));

                if(pos >= begin && pos <= end) {
                    if(p95 > maxYMaxEff)
                        maxYMaxEff = std::ceil(p95);
                    if(p5 < minYMaxEff)
                        minYMaxEff = std::floor(p5);
                }


                actPositions.push_back(pos);
                for(int mut=0; mut < 3; ++mut) {
                    double p95;
                    double p5;
                    if(mut == maxmutIdx) {
                        p95 = perc95Max.back();
                        p5 = perc5Max.back();
                        medians[mut].push_back(mediansMax.back());
                        perc95[mut].push_back(perc95Max.back());
                        perc5[mut].push_back(perc5Max.back());
                        perc75[mut].push_back(perc75Max.back());
                        perc25[mut].push_back(perc25Max.back());
                    } else {
                        p95 = log2(utils::getPercentile(data.totalRelKD_perPos[pos][mut], 95));
                        p5 = log2(utils::getPercentile(data.totalRelKD_perPos[pos][mut], 5));

                        medians[mut].push_back(log2(data.KDmedians[pos][mut]));
                        perc95[mut].push_back(p95);
                        perc5[mut].push_back(p5);
                        perc75[mut].push_back(log2(utils::getPercentile(data.totalRelKD_perPos[pos][mut], 75)));
                        perc25[mut].push_back(log2(utils::getPercentile(data.totalRelKD_perPos[pos][mut], 25)));
                    }
                    std::pair<double, double> iqr = utils::getxIQR(data.totalRelKD_perPos[pos][mut]);
                    minIQR[mut].push_back(log2(iqr.first));
                    maxIQR[mut].push_back(log2(iqr.second));

                    if(pos >= begin && pos <= end) {
                        if(p95 > maxY)
                            maxY = std::ceil(p95);
                        if(p5 < minY) {
                            minY = std::floor(p5);
                        }

                    }

                    int color = mut+1;
                    if(mut >= data.ref[pos]-1)
                        ++color;
                    colors[mut].push_back(color);
                    //use color for p-value star like for the boxplot, if not significant take white
                    if(data.pvalues[pos][mut] >= param.alpha || std::isnan(data.pvalues[pos][mut]))
                        color = 0;
                    else if(mut == maxmutIdx)
                        pValuesMaxEff.push_back(boost::make_tuple(pos, mediansMax.back(), color));
                    if(data.KDmedians[pos][mut] > 1)
                        posEffPValue[mut].push_back(std::make_pair(pos, color));
                    else
                        negEffPValue[mut].push_back(std::make_pair(pos, color));
                }
            }
            else
            {
                //if no (valid) maximal value -> plot position anyway but with 0.
                perc95Max.push_back(0);
                perc5Max.push_back(0);
                perc75Max.push_back(0);
                perc25Max.push_back(0);
                mediansMax.push_back(0);
                actPositionsMax.push_back(pos);
            }

            int refColor = data.ref[pos];
            refStr.push_back(boost::make_tuple(pos,ACGT[data.ref[pos]-1], refColor));
		}

//        for(int pos=param.seqBegin; pos <= param.seqEnd; ++pos)
//        {
//            int refColor = data.ref[pos];
//            refStr.push_back(boost::make_tuple(pos,ACGT[data.ref[pos]-1], refColor));
//        }

		
        //only show maximal y interval where data points are found
        if(yFrom < minY)
            yFrom = minY;
        if(yTo > maxY)
            yTo = maxY;

		Gnuplot* gp;
		fs::path* file;
        std::string fileStr;

        fs::path gpFile(resDir+"/tmp/relKdwtMut.gp");
        std::string gnuplotCall = ">"+gpFile.string();

        if(svg) {
            yRef = (yTo-yFrom)/4;
            file = new fs::path(resDir+"/tmp/relKdwtMut.svg");
            fileStr = (*file).string();
            delete file;
            //fs::path gpFile(resDir+"/tmp/relKdwtMut.gp");
            //gp = new Gnuplot("tee "+ gpFile.string()+" | gnuplot -persist");
            gp = new Gnuplot(gnuplotCall);

            *gp << "set output '" <<fileStr << "'\n";
            //set margins for multiplot: no gap between plot, enough space for axis labels
            *gp << "set tmargin 5\n";
			*gp << "set bmargin 0\n";
			
            *gp << "set term svg enhanced font 'Helvetica,20' size " << std::max(std::round((end-begin+1)/8), 5.0)*300 << " 3.5*300\n";
			
			//multiplot in window, with title not in plot, but in window (to still see it while scrolling)
			*gp << "set multiplot layout 2,1\n";
			*gp << "set format x ''\n";
			*gp << "set format y\n";

        }
        else if(eps) {
            yRef = (yTo-yFrom)/8;
            std::string fileName = "/relKdWtMut";
            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".eps");
            fileStr = (*file).string();
            delete file;
            //fs::path gpFile(resDir+"/tmp/relKdWtMut.gp");
            //gp = new Gnuplot("tee "+ gpFile.string()+" | gnuplot -persist");
            gp = new Gnuplot(gnuplotCall);

            *gp << "set output '" << fileStr << "'\n";
			//default size is 5 inchen x 3.5 inches and 40 positions fits perfect to this 
			*gp << "set term postscript eps enhanced color size "<< std::max(std::round((end-begin+1)/8), 5.0) <<  ", 3.5\n"; 
			//title and axis labels only in eps. for svg: show title in window
			*gp << "set title 'Relative binding affinity wt -> mut'\n";

            *gp << "set xlabel 'Sequence position'\n";

            //set margins for single plot: enough space for axis labels
            *gp << "set tmargin 5\n";
            *gp << "set bmargin 5\n";
			
		}
        else
        {
            yRef = (yTo-yFrom)/8;
            std::string fileName = "/relKdWtMut";
            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".pdf");
            fileStr = (*file).string();
            delete file;
            gp = new Gnuplot(gnuplotCall);

            *gp << "set output '" << fileStr << "'\n";
            //default size is 5 inchen x 3.5 inches and 40 positions fits perfect to this
            //*gp << "set term postscript eps enhanced color size "<< std::max(std::round((end-begin+1)/8), 5.0) <<  ", 3.5\n";
            *gp << "set term pdfcairo enhanced color size "<< std::max(std::round((end-begin+1)/8), 5.0) <<  ", 3.5 font 'Verdana, 8'\n";
            //title and axis labels only in eps. for svg: show title in window
            *gp << "set title 'Relative binding affinity wt -> mut'\n";

            *gp << "set xlabel 'Sequence position'\n";

            //set margins for single plot: enough space for axis labels
            *gp << "set tmargin 5\n";
            *gp << "set bmargin 5\n";
        }

        *gp << "set lmargin 10\n";
        *gp << "set rmargin 10\n";

        *gp << "set ylabel 'log_2(Kd_{mut}/Kd_{wt})\n";
        *gp << "set y2label 'log_2(Kd_{mut}/Kd_{wt})\n";
//		*gp << "set style fill solid 0.25\n";
		*gp << "set grid ytics linestyle 0\n";
        *gp << "set yrange [" << yFrom-(yRef*1.5) << ":" << yTo << "]\n";

		*gp << "set xtics 0,5," << param.seqEnd << "\n";
		*gp << "set xrange [" << begin-0.5 << ":" << end+0.5 << "]\n";
		
		*gp << "set boxwidth  0.1\n";
		*gp << "set pointsize 0.05\n";
		*gp << "set style fill solid\n";
		
		
		
// 		gp << "unset key\n";
		*gp << "unset colorbox\n";
		*gp << "set cbrange [0:4]\n";
		*gp << "set palette maxcolors 5\n";
		*gp << "set palette defined (0 'white', 1 'red', 2 'blue', 3 'green', 4 'orange')\n";
// 		gp << "set style fill solid 0.25 border -1\n";  
		
		*gp << "plot '-' using ($1-0.2):2:3:4:5:6 with candlestick notitle lt 1 lw 2 lc palette, ";
		*gp << "'-' using ($1-0.2):2:2:2:2 with candlestick notitle lt -1 lw 3 lc rgb 'black', ";
		*gp << "'-' using ($1):2:3:4:5:6 with candlestick notitle lt 1 lw 2 lc palette, ";
		*gp << "'-' using 1:2:2:2:2 with candlestick notitle lt -1 lw 3 lc rgb 'black', ";
		*gp << "'-' using ($1+0.2):2:3:4:5:6 with candlestick notitle lt 1 lw 2 lc palette, ";
		*gp << "'-' using ($1+0.2):2:2:2:2 with candlestick notitle lt -1 lw 3 lc rgb 'black', ";
		*gp << "'-' using 1:2 notitle w lines lt 2 lw 2 lc rgb 'black', ";
        *gp << "'-' using 1:(" << yFrom-yRef <<"):2:3 with labels notitle center tc palette, ";
        *gp << "'-' using ($1-0.2):(" << yFrom-yRef+yRef*0.2 << "):('*'):2 with labels notitle center tc palette font '"<< font <<"', ";
        *gp << "'-' using 1:(" << yFrom-yRef+yRef*0.2 << "):('*'):2 with labels notitle center tc palette font '"<< font <<"', ";
        *gp << "'-' using ($1+0.2):(" << yFrom-yRef+yRef*0.2 << "):('*'):2 with labels notitle center tc palette font '"<< font <<"', ";
        *gp << "'-' using ($1-0.2):(" << yFrom-yRef-yRef*0.3 << "):('*'):2 with labels notitle center tc palette font '"<< font <<"', ";
        *gp << "'-' using 1:(" << yFrom-yRef-yRef*0.3 << "):('*'):2 with labels notitle center tc palette font '"<< font <<"', ";
        *gp << "'-' using ($1+0.2):(" << yFrom-yRef-yRef*0.3 << "):('*'):2 with labels notitle center tc palette font '"<< font <<"'\n";
		// x boxmin whiskermin whiskermax boxmax
        (*gp).send1d(boost::make_tuple(actPositions, perc25[0], minIQR[0], maxIQR[0], perc75[0], colors[0]));
        (*gp).send1d(std::make_pair(actPositions,medians[0]));
        (*gp).send1d(boost::make_tuple(actPositions, perc25[1], minIQR[1], maxIQR[1], perc75[1], colors[1]));
        (*gp).send1d(std::make_pair(actPositions,medians[1]));
       (*gp).send1d(boost::make_tuple(actPositions, perc25[2], minIQR[2], maxIQR[2], perc75[2], colors[2]));
        (*gp).send1d(std::make_pair(actPositions,medians[2]));
// 		gp.send1d(std::make_pair(data.positions, nullLine));
// 		gp.send1d(std::make_pair(data.positions, refStr));
		(*gp).send1d(nullLine);
		(*gp).send1d(refStr);
		(*gp).send1d(posEffPValue[0]);
		(*gp).send1d(posEffPValue[1]);
		(*gp).send1d(posEffPValue[2]);
		(*gp).send1d(negEffPValue[0]);
		(*gp).send1d(negEffPValue[1]);
		(*gp).send1d(negEffPValue[2]);
		
        if(!svg)
        {
            callGnuplot(gpFile, resDir);
            gpFile = resDir+"/tmp/maxEffectOnKd.gp";
            gnuplotCall = ">"+gpFile.string();
        }

		/* max effect graph */

        if(yFrom < minYMaxEff)
            yFrom = minYMaxEff;
        if(yTo > maxYMaxEff)
            yTo = maxYMaxEff;

		if(svg) {
			*gp << "unset label\n";
            *gp << "unset point\n";
// 			*gp << "set label 1 'rel. binding affiniy mut/wt' at graph 1.0, graph 0.5 rotate by 90 center offset 1,0\n";
//			*gp << "set format x\n";
			*gp << "set tmargin 0\n";
			*gp << "set bmargin 5\n";
            *gp << "set xtics 0,5," << param.seqEnd << "\n";
            *gp << "set y2label 'log_{2}(Kd_{mut_{max}}/Kd_{wt})\n";
        }
        else if(eps) {
            std::string fileName = "/maxEffectOnKd";
            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".eps");
            fileStr = (*file).string();
            delete file;
            //fs::path gpFile(resDir+"/tmp/maxEffect.gp");
            //gp = new Gnuplot("tee " + gpFile.string()+" | gnuplot -persist");
            // delete gp object for all Kd values and create new one for max Kd values, if saving them
            delete gp;
            gp = new Gnuplot(gnuplotCall);
			
            *gp << "set output '" << fileStr << "'\n";
            //plot maxEffect as whole
//			*gp << "set term postscript eps enhanced color size "<< std::max(std::round((end-begin+1)/8), 5.0) <<  ", 3.5\n";
            *gp << "set term postscript eps enhanced color\n";
			*gp << "set title 'Relative binding affinity wt -> mut_{max}'\n";
			*gp << "set grid ytics linestyle 0\n";
			*gp << "unset colorbox\n";
			*gp << "set cbrange [0:4]\n";
			*gp << "set palette maxcolors 5\n";
            *gp << "set palette defined (0 'white', 1 'red', 2 'blue', 3 'green', 4 'orange')\n";
            *gp << "set label 'A' at graph 0.8, graph  0.95, 1 right tc palette\n";
            *gp << "set label 'C' at graph 0.85, graph  0.95, 2 right tc palette\n";
            *gp << "set label 'G' at graph 0.9, graph  0.95, 3 right tc palette\n";
            *gp << "set label 'U' at graph 0.95, graph  0.95, 4 right tc palette\n";

            //set margins for single plot: enough space for axis labels
            *gp << "set tmargin 5\n";
            *gp << "set bmargin 5\n";
		}
        else {
            std::string fileName = "/maxEffectOnKd";
            if(!filenamePrefix.empty())
                fileName += "_";
            file = new fs::path(resDir+"/plots"+fileName+filenamePrefix+".pdf");
            fileStr = (*file).string();
            delete file;
            //fs::path gpFile(resDir+"/tmp/maxEffect.gp");
            //gp = new Gnuplot("tee " + gpFile.string()+" | gnuplot -persist");
            // delete gp object for all Kd values and create new one for max Kd values, if saving them
            delete gp;
            gp = new Gnuplot(gnuplotCall);

            *gp << "set output '" << fileStr << "'\n";
            *gp << "set term pdfcairo enhanced color font 'Verdana, 8'\n";
            *gp << "set title 'Relative binding affinity wt -> mut_{max}'\n";
            *gp << "set grid ytics linestyle 0\n";
            *gp << "unset colorbox\n";
            *gp << "set cbrange [0:4]\n";
            *gp << "set palette maxcolors 5\n";
            *gp << "set palette defined (0 'white', 1 'red', 2 'blue', 3 'green', 4 'orange')\n";
            *gp << "set label 'A' at graph 0.8, graph  0.95, 1 right tc palette\n";
            *gp << "set label 'C' at graph 0.85, graph  0.95, 2 right tc palette\n";
            *gp << "set label 'G' at graph 0.9, graph  0.95, 3 right tc palette\n";
            *gp << "set label 'U' at graph 0.95, graph  0.95, 4 right tc palette\n";

            //set margins for single plot: enough space for axis labels
            *gp << "set tmargin 5\n";
            *gp << "set bmargin 5\n";
        }
        *gp << "set lmargin 10\n";
        *gp << "set rmargin 10\n";


         if(param.plotPValues && !svg)
         {
            if(!eps)
                *gp << "set pointsize 0.3\n";
            else
                *gp << "set pointsize 0.7\n";
         }
        *gp << "set xlabel 'Sequence position'\n";
        *gp << "set ylabel 'log_{2}(Kd_{mut_{max}}/Kd_{wt})'\n";
        *gp << "set yrange [" << yFrom << ":" << yTo << "]\n";;
        *gp << "set format x\n";

        *gp << "set xrange [" << begin-0.5 << ":" << end+0.5 << "]\n";
//        *gp << "set xrange [" << param.seqBegin-0.5 << ":" << param.seqEnd+0.5 << "]\n";
		*gp << "set style data lines\n";
        *gp << "plot '-' notitle w filledcu lc rgb 'grey90', ";
		*gp << "'-' notitle w filledcu lc rgb 'grey70', ";
		*gp << "'-' notitle w filledcu lc rgb 'grey70', ";
		*gp << "'-' notitle w filledcu lc rgb 'grey90', ";
        *gp << "'-' with lines lt 1 lc rgb 'black' lw 2 notitle, ";
        *gp << "'-' notitle w lines lt -1 lw 1.5 lc rgb 'black', ";
        if(param.plotPValues)
        {
            if(svg)
                //because the points will only be shown in black otherwise :(
                *gp << "'-' using 1:2:('*'):3 with labels notitle center nopoint offset 0, -0.25 tc palette font 'Helvetica,35'\n";
            else
                *gp << "'-' with points pt 3 lc palette notitle\n";
        }

		(*gp).send1d(boost::make_tuple(actPositionsMax, perc95Max, perc75Max));
		(*gp).send1d(boost::make_tuple(actPositionsMax, perc75Max, mediansMax));
		(*gp).send1d(boost::make_tuple(actPositionsMax, mediansMax, perc25Max));
		(*gp).send1d(boost::make_tuple(actPositionsMax, perc25Max, perc5Max));
		(*gp).send1d(std::make_pair(actPositionsMax, mediansMax));
        (*gp).send1d(nullLine);
        if(param.plotPValues)
            (*gp).send1d(pValuesMaxEff);

		if(svg) {
			*gp << "unset multiplot\n";
			*gp << "set output\n";
		}
        delete gp;
        callGnuplot(gpFile, resDir);
        return fileStr;
	}

    string plotAllEffects(const string& resDir, utils::Parameter& param, utils::DataContainer& data, plot::PlotFormat plotformat, const string &filenamePrefix) {
        bool svg = plotformat == plot::SVG;
        bool eps = plotformat == plot::EPS;
        string file = plotAllKd(resDir, param, data, svg, filenamePrefix, eps);
        return file;
    }

}
