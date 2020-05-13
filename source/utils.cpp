#include "utils.hpp"
#include <algorithm>
#include <iostream>
namespace utils {

    //TODO: unnötig, nur bound und unbound
	SampleType getSampleTypeFromString(std::string& s) {
		SampleType t = EMPTY;
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		if("dna"==s) t = DNA;
		else if("rna"==s) t = RNA; 
		else if("cdna"==s) t = cDNA;
		else if("bound"==s || "virus"==s) t = BOUND;
		else if("unbound"==s || "cell"==s) t=UNBOUND;
		return t;
	}
	
    double getMean(std::valarray<double>& v) {
        return v.sum()/v.size();
	}

    double getStd(std::valarray<double>& v) {
        double m = getMean(v);
        std::valarray<double> vNew = v-m;
        vNew *= vNew;

        return std::sqrt(vNew.sum()/v.size());
	}

    double getCoeffVar(std::valarray<double>& v) {
        return getStd(v)/getMean(v);
    }
	
	double getMedian(std::multiset<double>& s) {
		double res;
		int size = s.size();
		if(size % 2 == 0) {
			std::multiset<double>::iterator it1 = std::next(s.begin(), size/2);
			std::multiset<double>::iterator it2 = std::next(s.begin(),(size/2)-1);
			res = (*it1 + *it2)/2.f;
		} else {
			std::multiset<double>::iterator it = std::next(s.begin(),size/2);
			res = *it;
		}
		
		return res;
	}
	
	double getPercentile(std::multiset<double>& s, int p) {
		double res = std::numeric_limits<double>::quiet_NaN();
		int size = s.size();

		if(size > 0) {
			int idx1 = round(size*p/100.f);
			if(p == 50) {
				res = getMedian(s);
			} else if(p == 0) {
				res = *(s.begin());
			} else if(p == 100) {
				res = *(std::next(s.begin(), size-1));
			}else {
				int idx2 = idx1+1;
                std::multiset<double>::iterator it1 = std::next(s.begin(), std::max(idx1-1,0));
                std::multiset<double>::iterator it2 = std::next(s.begin(), std::min(idx2-1, size-1));
				
				
				double percentile1 = 100*((idx1-0.5)/(double)size);
				double percentile2 = 100*((idx2-0.5)/(double)size);
				
				double val1 = *it1;
				double val2 = *it2;
				double m =(val2-val1)/(percentile2-percentile1);
				res = val1 + m*(p - percentile1);
			}
		}
		return res;
	}
	
// 	template <typename T>
	double getPercentile(std::vector<double>& values, int percentile) {
		std::multiset<double> sortedValues;
		for(double entry : values) {
			if(entry > 0) 
				sortedValues.insert(entry);
		}
		double res = std::numeric_limits<double>::quiet_NaN();
		if(sortedValues.size() > 0)
			res = getPercentile(sortedValues, percentile);
		return res;
	}

    // for whiskers of the boxplot/ candele stick, instead of 5/95 percentile take 1.5 x +-IQR
    std::pair<double, double> getxIQR(std::vector<double>& values) {
        std::multiset<double> sortedValues;
        for(double entry : values) {
            if(entry > 0)
                sortedValues.insert(entry);
        }
        double minVal = std::numeric_limits<double>::quiet_NaN();
        double maxVal = std::numeric_limits<double>::quiet_NaN();

        if(sortedValues.size() > 0) {
            double q1 = getPercentile(sortedValues, 25);
            double q3 = getPercentile(sortedValues, 75);
            double iqr = q3-q1;
            //Returns an iterator pointing to the first element in the range [first,last) which does not compare less than val
            minVal = *(std::lower_bound(sortedValues.begin(), sortedValues.end(), q1-(1.5*iqr)));
            //Returns an iterator pointing to the first element in the range [first,last) which compares greater than val
            auto maxValItr = std::upper_bound(sortedValues.begin(), sortedValues.end(), q3+(1.5*iqr));
            if(maxValItr!=sortedValues.begin())
                maxValItr--;
            maxVal = *maxValItr;
        }

        return std::make_pair(minVal, maxVal);
    }
	
	
    //not possible to check for <= because of strict weak ordering
	std::vector<size_t> sort_indexes(std::vector<double>& v) {

		// initialize original index locations
		std::vector<size_t> idx(v.size());
		for(size_t i = 0; i != idx.size(); ++i) 
			idx[i] = i;

		// sort indexes based on comparing values in v
		sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) { return v[i1] < v[i2];});

		return idx;
	}

}
