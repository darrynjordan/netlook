#include "data.hpp"

radarData::radarData(int dataId)
{
	switch(dataId)
	{
		case D1: 
		{
			dataSetName = "raw_data/radar_data/e10_10_07_1421_30_P1_1_130000_S0_1_2047_node3.bin";
			refSigName  = "raw_data/reference_signals/refSigN3N3Pl600.txt";
			refSigSize  = 60;
			paddedSize  = 2304;
			break;
		}

		case D2: 
		{
			dataSetName = "raw_data/radar_data/e10_10_16_1800_40_P1_1_130000_S0_1_2047_node3.bin";
			refSigName  = "raw_data/reference_signals/refSigN3N3Pl1000.txt";
			refSigSize  = 100;
			paddedSize  = 2304;
			break;
		}

		case D3: 
		{
			dataSetName = "raw_data/radar_data/e10_10_21_1927_55_P1_1_130000_S0_1_2047_node3.bin";
			refSigName  = "raw_data/reference_signals/refSigN3N3Pl5000.txt";
			refSigSize  = 500;
			paddedSize  = 2592;
			break;
		}
			
		case D4: 
		{
			dataSetName = "raw_data/radar_data/e11_04_24_1139_36_P1_1_130000_S0_1_2047_node3.bin";
			refSigName  = "raw_data/reference_signals/refSigN3N3Pl6000.txt";
			refSigSize  = 600;	
			paddedSize  = 3072;
			break;
		}

		case D5: 
		{
			dataSetName = "raw_data/radar_data/e11_04_27_1300_13_P1_1_130000_S0_1_2047_node3.bin";
			refSigName  = "raw_data/reference_signals/refSigN3N3Pl10000.txt";
			refSigSize  = 1000;
			paddedSize  = 3072;
			break;
		}
	}		
}


