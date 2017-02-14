#ifndef DATA_HPP
#define DATA_HPP

#include "main.hpp"

class dataset
{
	private:
		const char* dataSetName;
		const char* refSigName;
		int	refSigSize;
		int	paddedSize;

	public:
		dataset(void);		
		void setID(int id);
		const char* getDataSetName(void){return dataSetName;}
		const char* getRefSigName(void){return refSigName;}
		int	getRefSigSize(void){return refSigSize;}
		int getPaddedSize(void){return paddedSize;}
};

#endif
