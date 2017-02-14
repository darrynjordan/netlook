#ifndef DATA_HPP
#define DATA_HPP

#include "main.hpp"

enum data {D1, D2, D3, D4, D5};

class radarData
{
	private:
		const char* dataSetName;
		const char* refSigName;
		int			refSigSize;
		int			paddedSize;

		void setGlobalSizes(int refSigSize, int paddedRangeSize);		

	public:
		radarData(int dataId);		
		const char* getDataSetName(void){return dataSetName;}
		const char* getRefSigName(void){return refSigName;}
		int 		getRefSigSize(void){return refSigSize;}
		int 		getPaddedSize(void){return paddedSize;}
};

#endif
