//	Pulse Compressor and Doppler Processor for NetRAD
//	written by: Darryn Jordan
// 	email: jrddar001@myuct.ac.za


#include "main.hpp"

//=====================================================================================================
// Global Variables
//=====================================================================================================

int		dopplerDataStart = 0; 

bool 	isDoppler = false;
bool 	isSuggestions = false;
bool	isVisualiser = false;
bool 	isZeroPad = false;		//%TODO implement this flag


//%TODO move data import into a function
radarData netrad(D2);

int REFSIZE 			= netrad.getRefSigSize();
int PADRANGESIZE 		= netrad.getPaddedSize();

int DOPPLERSIZE 		= 64;
int UPDATELINE 			= 13000;
int RANGESIZE 			= 2048;
int RANGELINES 			= 130000;
int FFTW_THREADS		= 1;
int THREADS				= 1;

int RANGELINESPERTHREAD = RANGELINES/THREADS;

//=====================================================================================================
// Allocate Memory
//=====================================================================================================

uint16_t *realDataBuffer;
double   *realRangeBuffer;
double   *realRefBuffer;
uint8_t  *dopplerImageBuffer;

fftw_complex *fftRangeBuffer;
fftw_complex *fftRefBuffer; 
fftw_complex *hilbertBuffer;
fftw_complex *dopplerBuffer;

//fftw_complex *dopplerData;

float *refWindow;
float *doppWindow;

//=====================================================================================================
// Setup FFTW Plans
//=====================================================================================================

fftw_plan refPlan;
fftw_plan hilbertPlan;
fftw_plan dopplerPlan;	

boost::mutex mutex;

int main(int argc, char *argv[])
{
	int opt;	
	while ((opt = getopt(argc, argv, "vt:c:")) != -1 ) 
    {
        switch (opt) 
        {
            case 'v':
                isVisualiser = true;
                break;
            case 't':
				THREADS = atoi(optarg);
				RANGELINESPERTHREAD = RANGELINES/THREADS;
				break;	
			case 'c':
				waterfallColourMapSlider = atoi(optarg);
				break;				
            case '?':
				if (optopt == 't')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);		
				else
				fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);				
        }
    }
    
    initTerminal();	
    allocateMemory();
    initMat();	
    
	boost::thread_group threadGroup;
	boost::thread threads[THREADS];			
	fftw_init_threads();
	fftw_plan_with_nthreads(FFTW_THREADS);
	fftw_make_planner_thread_safe();

	popLookUpTables();
	
	if (isVisualiser) initPlots();	
	
	loadRefData();	
	loadRangeData();
	
	startTime();
	
	fftw_execute(refPlan);	
	complxConjRef();	
	
	for (int i = 0; i < THREADS; i++)
		threadGroup.create_thread(boost::bind(&perThread, i));
	
	threadGroup.join_all();		
	
	if (isVisualiser) plotWaterfall();	
	
	endTime();	
	
	savePlots();
	//saveData();
	freeMem();
	cv::waitKey(0);
	
	return 0;
}


void perThread(int id)
{
	int start_index = id*RANGELINESPERTHREAD;
	
	fftw_plan rangePlan  = fftw_plan_dft_r2c_1d(PADRANGESIZE, &realRangeBuffer[id*PADRANGESIZE], &fftRangeBuffer[id*(PADRANGESIZE/2 + 1)], FFTW_MEASURE);
	fftw_plan resultPlan = fftw_plan_dft_c2r_1d(PADRANGESIZE, &hilbertBuffer[id*PADRANGESIZE], &realRangeBuffer[id*PADRANGESIZE], FFTW_MEASURE | FFTW_PRESERVE_INPUT);
	
	for (int i = start_index; i < start_index + RANGELINESPERTHREAD; i++)
	{		
		popRangeBuffer(i, &realRangeBuffer[id*PADRANGESIZE]);		
		fftw_execute(rangePlan);	
		
		//complex multiplication
		for (int j = 0; j < (PADRANGESIZE/2 + 1); j++)
		{			
			int k = j + id*(PADRANGESIZE);
			int l = j + id*(PADRANGESIZE/2 +1);
			
			hilbertBuffer[k][0] = fftRangeBuffer[l][0]*fftRefBuffer[j][0] - fftRangeBuffer[l][1]*fftRefBuffer[j][1];
			hilbertBuffer[k][1] = fftRangeBuffer[l][0]*fftRefBuffer[j][1] + fftRangeBuffer[l][1]*fftRefBuffer[j][0];
		}	
		
		fftw_execute(resultPlan);	
		
		updateWaterfall(i, &realRangeBuffer[id*PADRANGESIZE]);	
		
		if (isVisualiser && (i%UPDATELINE == 0))
		{
			mutex.lock();
			plotWaterfall();
			mutex.unlock();		
		}	

		/*if (doppOn)
		{
			hilbertTransform();					
			popDopplerData(i); 		
			processDoppler(i);		
		}	*/				
	}	
		
	fftw_destroy_plan(rangePlan);
	fftw_destroy_plan(resultPlan);	
	
	//std::cout << "(thread "<< id << ") Average Time per Line: "<< std::setprecision (2) << std::fixed << getTime()/RANGELINES * 1000000 << " us" << std::endl;
}


void allocateMemory(void)
{
	realDataBuffer = (uint16_t*)malloc(RANGELINES*RANGESIZE*sizeof(uint16_t));
	realRangeBuffer 	= (double*)malloc(THREADS*PADRANGESIZE*sizeof(double));
	realRefBuffer       = (double*)malloc(THREADS*PADRANGESIZE*sizeof(double));
	dopplerImageBuffer  = (uint8_t*)malloc(THREADS*DOPPLERSIZE*sizeof(uint8_t));

	fftRangeBuffer  = (fftw_complex*)malloc(THREADS*(PADRANGESIZE/2 + 1)*sizeof(fftw_complex));
	fftRefBuffer    = (fftw_complex*)malloc(THREADS*(PADRANGESIZE/2 + 1)*sizeof(fftw_complex)); 
	hilbertBuffer   = (fftw_complex*)malloc(THREADS*PADRANGESIZE*sizeof(fftw_complex));
	dopplerBuffer   = (fftw_complex*)malloc(THREADS*DOPPLERSIZE*sizeof(fftw_complex));
	//dopplerData     = (fftw_complex*)malloc((PADRANGESIZE*DOPPLERSIZE)*sizeof(fftw_complex));

	refWindow  = (float*)malloc(REFSIZE*sizeof(float));
	doppWindow = (float*)malloc(DOPPLERSIZE*sizeof(float));
	
	refPlan = fftw_plan_dft_r2c_1d(PADRANGESIZE, realRefBuffer, fftRefBuffer, FFTW_ESTIMATE);
	hilbertPlan = fftw_plan_dft_1d(PADRANGESIZE, hilbertBuffer, hilbertBuffer, FFTW_BACKWARD, FFTW_MEASURE);
	dopplerPlan = fftw_plan_dft_1d(DOPPLERSIZE, dopplerBuffer, dopplerBuffer, FFTW_FORWARD, FFTW_MEASURE);	
	
	std::cout << "Allocated Global Memory" << std::endl;
}


void processDoppler(int rangeLine)
{
	if ((rangeLine + 1 - dopplerDataStart) == DOPPLERSIZE)  //check that dopplerData is full
	{
		for (int i = 0; i < PADRANGESIZE; i++)		
		{
			popDopplerBuffer(i);	
			fftDopplerData();
			postProcessDoppler();
			updateDoppler(dopplerImageBuffer);	
		}
		plotDoppler();
	}
}

void popDopplerData(int rangeLine)
{
	/*if (rangeLine%UPDATELINE == 0)
		dopplerDataStart = rangeLine;

	if ((rangeLine + 1 - dopplerDataStart) <= DOPPLERSIZE)
	{
		for (int j = 0; j < PADRANGESIZE; j++)
		{
			dopplerData[j*DOPPLERSIZE + (rangeLine - dopplerDataStart)][0] = hilbertBuffer[j][0];
			dopplerData[j*DOPPLERSIZE + (rangeLine - dopplerDataStart)][1] = hilbertBuffer[j][1];
		}
	}*/
}

void popDopplerBuffer(int dopplerLine)
{
	/*for (int j = 0; j < DOPPLERSIZE; j++)
	{	
		dopplerBuffer[j][0] = dopplerData[dopplerLine*DOPPLERSIZE + j][0]*doppWindow[j]; 
		dopplerBuffer[j][1] = dopplerData[dopplerLine*DOPPLERSIZE + j][1]*doppWindow[j];
	}*/
}


void postProcessDoppler(void)
{
	float maxResult = 0.0f;	
	float result = 0.0f;
	int processed = 0;

	for (int i = 0; i < DOPPLERSIZE; i++)
	{
		result = (sqrt(dopplerBuffer[i][0]*dopplerBuffer[i][0] + dopplerBuffer[i][1]*dopplerBuffer[i][1]));

		if (result > maxResult)
			maxResult = result;
	}

	for (int i = 0; i < DOPPLERSIZE; i++)
	{
		processed = (uint8_t)(((sqrt(dopplerBuffer[i][0]*dopplerBuffer[i][0] + dopplerBuffer[i][1]*dopplerBuffer[i][1]))/maxResult)*255);

		if (processed < dopplerThresholdSlider) //set threshold
			processed = 0;
		
		if (i < (DOPPLERSIZE/2 + 1))		
			dopplerImageBuffer[i + (DOPPLERSIZE/2 - 1)] = processed;
		else
			dopplerImageBuffer[i - (DOPPLERSIZE/2 + 1)] = processed;
	}
}

void complxConjRef(void)
{
	for (int i = 0; i < (PADRANGESIZE/2 + 1); i++)
		fftRefBuffer[i][1] = -1*fftRefBuffer[i][1];
	//printMsg("Complex Conjugate Reference");
}

void popRangeBuffer(int rangeLine, double* realRangeBuffer)
{
	int start = rangeLine*RANGESIZE;

	//populate complex range data and remove offset	
	for (int i = 0; i < PADRANGESIZE; i++)
	{
		if (i < RANGESIZE)
			realRangeBuffer[i] = (double)(realDataBuffer[i + start] - ADCOFFSET);
		else
			realRangeBuffer[i] = 0; 
	}
}

void loadRangeData(void)
{
	FILE *dataBinFile_p;	
	
	//open binary file
	dataBinFile_p = fopen(netrad.getDataSetName(), "r");

	//read from binary files into buffers
	fread(realDataBuffer, sizeof(uint16_t), RANGELINES*RANGESIZE, dataBinFile_p);

	//close binary files
	fclose(dataBinFile_p);

	std::cout << "Range Data Loaded" << std::endl;
}

void loadRefData(void)
{
	std::vector<double> refVector;	
	std::ifstream file(netrad.getRefSigName());
    std::string line; 

    while (std::getline(file, line))
    {
		std::stringstream ss(line);

		double i;
		while (ss >> i)
		{
			refVector.push_back(i);

			if (ss.peek() == ',')
				ss.ignore();
		}
	}

	//populate complex ref data and window
	for (int i = 0; i < PADRANGESIZE; i++)
	{		
		if (i < REFSIZE)
			realRefBuffer[i] = (float)((-1*refVector[i])*refWindow[i]);
		else
			realRefBuffer[i] = 0;
	}

	std::cout << "Reference Data Loaded" << std::endl;	

}

void normRefData(void)
{
	float normFactor = getNormFactor();	
	
	for (int i = 0; i < PADRANGESIZE; i++)
		realRefBuffer[i] = realRefBuffer[i]/normFactor;
	printMsg("Reference Data Normalized");
}

float getNormFactor(void)
{
	float refMax = 0;	
	for(int i = 0; i < REFSIZE; i++)
	{
		if (realRefBuffer[i] > refMax)
			refMax = realRefBuffer[i];
	}
	
	return refMax;
}

void freeMem(void)
{
	fftw_free(fftRefBuffer);
	fftw_free(hilbertBuffer);
	fftw_free(fftRangeBuffer);
	//fftw_free(dopplerBuffer);
	//fftw_free(dopplerData);

	free(realDataBuffer);	
	free(realRefBuffer);
	free(realRangeBuffer);		
	
	fftw_destroy_plan(hilbertPlan);	
	fftw_destroy_plan(dopplerPlan);
	fftw_destroy_plan(refPlan);

	//printMsg("Memory Free \n");
}



