//	Pulse Compressor and Doppler Processor for NetRAD
//	written by: Darryn Jordan
// 	email: jrddar001@myuct.ac.za

#include "main.hpp"

//=====================================================================================================
// Global Variables
//=====================================================================================================
bool isDoppler = true;
bool isSuggestions = false;
bool isVisualiser = false;

int DATASETID = 0;
int REFSIZE = 100;
int PADRANGESIZE = 2048;
int DOPPLERSIZE = 128;
int UPDATELINE = 1000;
int RANGESIZE = 2048;
int RANGELINES = 130000;
int FFTW_THREADS = 1;
int THREADS = 1;
int RANGELINESPERTHREAD = RANGELINES/THREADS;
int REPETITIONS = 1;
int PLANNER_FLAG = 64;

dataset Dataset;

struct timeval time_struct;
double start_time, end_time;
double min_time, max_time, trial_time;
double avg_time = 0;

//=====================================================================================================
// Allocate Memory
//=====================================================================================================

uint16_t *realDataBuffer;
double   *realRangeBuffer;
double   *realRefBuffer;
double   *dopplerImageBuffer;

fftw_complex *fftRangeBuffer;
fftw_complex *fftRefBuffer; 
fftw_complex *hilbertBuffer;
fftw_complex *dopplerBuffer;
fftw_complex *dopplerData;

float *refWindow;
float *rangeWindow;
float *doppWindow;

//=====================================================================================================
// Setup FFTW Plans
//=====================================================================================================

fftw_plan refPlan;

boost::mutex mutex;

int main(int argc, char *argv[])
{
	int opt;	
	while ((opt = getopt(argc, argv, "hp:r:f:u:l:z:d:vt:c:D:")) != -1) 
    {
        switch (opt) 
        {
            case 'h':
				help();						
				break;
            case 'd':
				DATASETID = atoi(optarg);				
				break;
			case 'f':
				FFTW_THREADS = atoi(optarg);
				break;
			case 'p':
				PLANNER_FLAG = atoi(optarg);	
				break;
			case 'z':
				PADRANGESIZE = atoi(optarg);
				break;
			case 'r':
				REPETITIONS = atoi(optarg);
				break;
			case 'D':
				DOPPLERSIZE = atoi(optarg);
				break;
			case 'u':
				UPDATELINE = atoi(optarg);
				break;
			case 'l':
				RANGELINES = atoi(optarg);
				RANGELINESPERTHREAD = RANGELINES/THREADS;
				break;
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
    
    Dataset.setID(DATASETID);					
	REFSIZE = Dataset.getRefSigSize();    

	initTerminal();	
    
	processingLoop(REPETITIONS);	
	
	timingSummary();
	
	if (isVisualiser)
	{
		savePlots();
		cv::waitKey(0);
	}
	
	return 0;
}

void processingLoop(int repetitions)
{
	boost::thread_group threadGroup;
	
	for (int j = 0; j < repetitions; j ++)
	{	
		allocateMemory();
		
		boost::thread threads[THREADS];			
		fftw_init_threads();
		fftw_plan_with_nthreads(FFTW_THREADS);
		fftw_make_planner_thread_safe();
		
		popLookUpTables();		
		
		initMats();
		
		if (isVisualiser) 
		{
			initPlots();	
		}
		
		loadRefData();	
		loadRangeData();
		
		restartTimer();
		
		fftw_execute(refPlan);	
		complxConjRef();	
		
		for (int i = 0; i < THREADS; i++)
		{
			threadGroup.create_thread(boost::bind(&perThread, i));
		}
		
		threadGroup.join_all();		
		
		trial_time = getTimeElapsed();
		
		if (isVisualiser) 
		{
			plotWaterfall();	
		}
	
		//saveData();
		freeMemory();
		
		printf("Trial %i: %.5f s\n", j, trial_time);
		
		if (j == 0)
		{
			min_time = trial_time;
			max_time = trial_time;			
		}
		else
		{
			if (trial_time < min_time)
				min_time = trial_time;
			if (trial_time > max_time)
				max_time = trial_time;
		}
		
		avg_time += trial_time;
	}
	
	if (repetitions > 1)
	{
		avg_time = avg_time/repetitions;
		
		std::cout << std::endl;
		printf("Min: %.5f s\n", min_time);
		printf("Max: %.5f s\n", max_time);
		printf("Avg: %.5f s\n", avg_time);		
	}
	
	std::cout << std::endl;
}


void perThread(int id)
{
	int range_start_index = id*RANGELINESPERTHREAD;
	int	doppler_start_index = id*RANGELINESPERTHREAD; 
	
	fftw_plan rangePlan  = fftw_plan_dft_r2c_1d(PADRANGESIZE, &realRangeBuffer[id*PADRANGESIZE], &fftRangeBuffer[id*(PADRANGESIZE/2 + 1)], PLANNER_FLAG);
	fftw_plan resultPlan = fftw_plan_dft_c2r_1d(PADRANGESIZE, &hilbertBuffer[id*PADRANGESIZE], &realRangeBuffer[id*PADRANGESIZE], PLANNER_FLAG | FFTW_PRESERVE_INPUT);
	fftw_plan hilbertPlan = fftw_plan_dft_1d(PADRANGESIZE, &hilbertBuffer[id*PADRANGESIZE], &hilbertBuffer[id*PADRANGESIZE], FFTW_BACKWARD, PLANNER_FLAG);
	fftw_plan dopplerPlan = fftw_plan_dft_1d(DOPPLERSIZE, &dopplerBuffer[id*DOPPLERSIZE], &dopplerBuffer[id*DOPPLERSIZE], FFTW_FORWARD, PLANNER_FLAG);	
	
	for (int rangeLine = range_start_index; rangeLine < range_start_index + RANGELINESPERTHREAD; rangeLine++)
	{
		popRangeBuffer(rangeLine, &realRangeBuffer[id*PADRANGESIZE]);	
		
		fftw_execute(rangePlan);	
		
		//complex multiplication
		for (int j = 0; j < (PADRANGESIZE/2 + 1); j++)
		{			
			int k = j + id*(PADRANGESIZE);
			int l = j + id*(PADRANGESIZE/2 + 1);
			
			hilbertBuffer[k][0] = fftRangeBuffer[l][0]*fftRefBuffer[j][0] - fftRangeBuffer[l][1]*fftRefBuffer[j][1];
			hilbertBuffer[k][1] = fftRangeBuffer[l][0]*fftRefBuffer[j][1] + fftRangeBuffer[l][1]*fftRefBuffer[j][0];
		}	
		
		fftw_execute(resultPlan);	
		
		updateWaterfall(rangeLine, &realRangeBuffer[id*PADRANGESIZE]);	
		
		if (isVisualiser && (rangeLine%UPDATELINE == 0))
		{
			mutex.lock();
			plotWaterfall();
			mutex.unlock();		
		}	

		if (isDoppler)
		{
			//reached a multiple of UPDATELINE, new start point for collecting Doppler data
			if (rangeLine%UPDATELINE == 0)
			{
				doppler_start_index = rangeLine;
			}

			//is the current rangeLine within DOPPLERSIZE from doppler_start_index
			if ((rangeLine - doppler_start_index + 1) <= DOPPLERSIZE)
			{
				//zero the positive half of the pulse compressed spectrum
				memset(&hilbertBuffer[(id*PADRANGESIZE) + (PADRANGESIZE/2 + 1)], 0,  sizeof(fftw_complex)*(PADRANGESIZE/2 - 1));	
				fftw_execute(hilbertPlan);
				
				for (int j = 0; j < PADRANGESIZE; j++)
				{
					dopplerData[id*PADRANGESIZE*DOPPLERSIZE + j*DOPPLERSIZE + (rangeLine - doppler_start_index)][0] = hilbertBuffer[id*PADRANGESIZE + j][0];
					dopplerData[id*PADRANGESIZE*DOPPLERSIZE + j*DOPPLERSIZE + (rangeLine - doppler_start_index)][1] = hilbertBuffer[id*PADRANGESIZE + j][1];
				}
			}			
				
			//check if dopplerData is full		
			if ((rangeLine - doppler_start_index + 1) == DOPPLERSIZE)  
			{
				//begin Doppler processing
				for (int dopplerLine = 0; dopplerLine < PADRANGESIZE; dopplerLine++)		
				{
					//isolate a Doppler line
					for (int j = 0; j < DOPPLERSIZE; j++)
					{	
						dopplerBuffer[id*DOPPLERSIZE + j][0] = dopplerData[id*PADRANGESIZE*DOPPLERSIZE + dopplerLine*DOPPLERSIZE + j][0]*doppWindow[j]; 
						dopplerBuffer[id*DOPPLERSIZE + j][1] = dopplerData[id*PADRANGESIZE*DOPPLERSIZE + dopplerLine*DOPPLERSIZE + j][1]*doppWindow[j];
					}
					
					//FFT Doppler line
					fftw_execute(dopplerPlan);
					
					if (isVisualiser)
					{
						postProcessDoppler(id);					
						updateDoppler(id, &dopplerImageBuffer[id*DOPPLERSIZE]);	
					}
				}
				
				if (isVisualiser)
				{
					mutex.lock();
					plotDoppler(id);
					mutex.unlock();	
				}			
			}	
		}					
	}	
	
	fftw_destroy_plan(rangePlan);
	fftw_destroy_plan(resultPlan);	
	fftw_destroy_plan(hilbertPlan);	
	fftw_destroy_plan(dopplerPlan);
}


void allocateMemory(void)
{
	realDataBuffer = (uint16_t*)malloc(RANGELINES*RANGESIZE*sizeof(uint16_t));
	realRangeBuffer 	= (double*)malloc(THREADS*PADRANGESIZE*sizeof(double));
	realRefBuffer       = (double*)malloc(THREADS*PADRANGESIZE*sizeof(double));
	dopplerImageBuffer  = (double*)malloc(THREADS*DOPPLERSIZE*sizeof(double));

	fftRangeBuffer  = (fftw_complex*)malloc(THREADS*(PADRANGESIZE/2 + 1)*sizeof(fftw_complex));
	fftRefBuffer    = (fftw_complex*)malloc(THREADS*(PADRANGESIZE/2 + 1)*sizeof(fftw_complex)); 
	hilbertBuffer   = (fftw_complex*)malloc(THREADS*PADRANGESIZE*sizeof(fftw_complex));
	dopplerBuffer   = (fftw_complex*)malloc(THREADS*DOPPLERSIZE*sizeof(fftw_complex));
	dopplerData     = (fftw_complex*)malloc((THREADS*PADRANGESIZE*DOPPLERSIZE)*sizeof(fftw_complex));

	refWindow  = (float*)malloc(REFSIZE*sizeof(float));
	doppWindow = (float*)malloc(DOPPLERSIZE*sizeof(float));
	rangeWindow = (float*)malloc(RANGESIZE*sizeof(float));
	
	refPlan = fftw_plan_dft_r2c_1d(PADRANGESIZE, realRefBuffer, fftRefBuffer, PLANNER_FLAG);
	
	//std::cout << "Allocated Global Memory" << std::endl;
}


void postProcessDoppler(int thread_id)
{
	float maxResult = 0.0f;	
	float result = 0.0f;
	int processed = 0;
	int index = 0;

	for (int i = 0; i < DOPPLERSIZE; i++)
	{
		index = thread_id*DOPPLERSIZE + i;
		result = sqrt(dopplerBuffer[index][0]*dopplerBuffer[index][0] + dopplerBuffer[index][1]*dopplerBuffer[index][1]);

		if (result > maxResult)
			maxResult = result;
	}

	for (int i = 0; i < DOPPLERSIZE; i++)
	{
		index = thread_id*DOPPLERSIZE + i;
		processed = ((sqrt(dopplerBuffer[index][0]*dopplerBuffer[index][0] + dopplerBuffer[index][1]*dopplerBuffer[index][1]))/maxResult)*255;
		
		if (i < (DOPPLERSIZE/2 + 1))		
			dopplerImageBuffer[index + (DOPPLERSIZE/2 - 1)] = processed;
		else
			dopplerImageBuffer[index - (DOPPLERSIZE/2 + 1)] = processed;
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
			realRangeBuffer[i] = (double)(realDataBuffer[i + start] - ADCOFFSET)*rangeWindow[i];
		else
			realRangeBuffer[i] = 0; 
	}
}

void loadRangeData(void)
{
	FILE *dataBinFile_p;	
	
	//open binary file
	dataBinFile_p = fopen(Dataset.getDataSetName(), "r");

	//read from binary files into buffers
	fread(realDataBuffer, sizeof(uint16_t), RANGELINES*RANGESIZE, dataBinFile_p);

	//close binary files
	fclose(dataBinFile_p);

	//std::cout << "Range Data Loaded" << std::endl;
}

void loadRefData(void)
{
	std::vector<double> refVector;	
	std::ifstream file(Dataset.getRefSigName());
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

	//std::cout << "Reference Data Loaded" << std::endl;	
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

void freeMemory(void)
{
	fftw_free(fftRefBuffer);
	fftw_free(hilbertBuffer);
	fftw_free(fftRangeBuffer);
	fftw_free(dopplerBuffer);
	fftw_free(dopplerData);

	free(realDataBuffer);	
	free(realRefBuffer);
	free(realRangeBuffer);		
	
	fftw_destroy_plan(refPlan);

	//printMsg("Memory Free \n");
}

double getTimeElapsed(void)
{	
    gettimeofday(&time_struct, NULL);
	end_time = (double)time_struct.tv_sec + (double)(time_struct.tv_usec*1e-6);
	return (double)(end_time - start_time);
}

void restartTimer(void)
{
    gettimeofday(&time_struct, NULL);
	start_time = (double)time_struct.tv_sec + (double)(time_struct.tv_usec*1e-6);	
}


