#pragma once
#include <string>
#include "utils.hpp"
using namespace std;

namespace plot {

    enum PlotFormat {
        SVG,
        EPS,
        PDF
    };
	
    string plotMedianErrorPerSample(const string& resDir, utils::Parameter& param, utils::DataContainer& data, PlotFormat plotformat, const string& filenamePrefix = "");
	
    string plotCoefficientOfVariation(const string& resDir, utils::Parameter& param, utils::DataContainer& data, PlotFormat plotformat, const string& filenamePrefix = "");
	
    string plotMutationRatePerSampleBoxplot(const string& resDir, const string& expDir, utils::Parameter& param, utils::DataContainer& data, PlotFormat plotformat, const string& filenamePrefix = "");
	
    string plotAllEffects(const string& resDir, utils::Parameter& param, utils::DataContainer& data, plot::PlotFormat plotformat, const string& filenamePrefix = "");

}
