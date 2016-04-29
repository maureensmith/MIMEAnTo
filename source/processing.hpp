#pragma once

#include "utils.hpp"
#include "ioTools.hpp"


using namespace std;

namespace processing {

	void rewriteDatafiles(const string& s, utils::refMap& ref);
	
	void estimateError(const string& expDir, utils::Parameter& param, utils::DataContainer& data);
		
	void computeKDvalues(const string& expDir, utils::Parameter& param, utils::DataContainer& data);
		
	void applyQualityCriteria(utils::Parameter& param, utils::DataContainer& data);
	
}
