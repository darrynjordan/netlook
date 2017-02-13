//	Pulse Compressor and Doppler Processor for NetRAD
//	written by: Darryn Jordan
// 	email: jrddar001@myuct.ac.za


#include "main.hpp"

//=====================================================================================================
// Global Variables
//=====================================================================================================

double 		tStart, tEnd;	
int			dopplerDataStart = 0; 
int			dopplerThresholdSlider = 0;
bool 		doppOn = false;
bool 		suggestOn = false;

radarData netrad(D5);

int REFSIZE 			= netrad.getRefSigSize();
int PADRANGESIZE 		= netrad.getPaddedSize();

int DOPPLERSIZE 		= 64;
int UPDATELINE 			= 100;
int RANGESIZE 			= 2048;
int RANGELINES 			= 130000;
int FFTW_THREADS		= 1;
int THREADS				= 2;

int RANGELINESPERTHREAD = RANGELINES/THREADS;

//=====================================================================================================
// Allocate Memory
//=====================================================================================================

uint16_t *realDataBuffer      = (uint16_t*)malloc(RANGELINES*RANGESIZE*sizeof(uint16_t));

double   *realRangeBuffer 	  = (double*)malloc(THREADS*PADRANGESIZE*sizeof(double));
double   *realRefBuffer       = (double*)malloc(THREADS*PADRANGESIZE*sizeof(double));

uint8_t  *dopplerImageBuffer  = (uint8_t*)malloc(THREADS*DOPPLERSIZE*sizeof(uint8_t));

fftw_complex *fftRangeBuffer  = (fftw_complex*)malloc(THREADS*(PADRANGESIZE/2 + 1)*sizeof(fftw_complex));
fftw_complex *fftRefBuffer    = (fftw_complex*)malloc(THREADS*(PADRANGESIZE/2 + 1)*sizeof(fftw_complex)); 

fftw_complex *hilbertBuffer   = (fftw_complex*)malloc(THREADS*PADRANGESIZE*sizeof(fftw_complex));
fftw_complex *dopplerBuffer   = (fftw_complex*)malloc(THREADS*DOPPLERSIZE*sizeof(fftw_complex));

//fftw_complex *dopplerData     = (fftw_complex*)malloc((PADRANGESIZE*DOPPLERSIZE)*sizeof(fftw_complex));

float *refWindow			  = (float*)malloc(REFSIZE*sizeof(float));
float *doppWindow			  = (float*)malloc(DOPPLERSIZE*sizeof(float));

//=====================================================================================================
// Setup FFTW Plans
//=====================================================================================================

fftw_plan refPlan = fftw_plan_dft_r2c_1d(PADRANGESIZE, realRefBuffer, fftRefBuffer, FFTW_ESTIMATE);
fftw_plan hilbertPlan = fftw_plan_dft_1d(PADRANGESIZE, hilbertBuffer, hilbertBuffer, FFTW_BACKWARD, FFTW_MEASURE);
fftw_plan dopplerPlan = fftw_plan_dft_1d(DOPPLERSIZE, dopplerBuffer, dopplerBuffer, FFTW_FORWARD, FFTW_MEASURE);	

boost::mutex mutex;

int main(int argc, char *argv[])
{
	boost::thread_group threadGroup;
	boost::thread threads[THREADS];		
	
	initTerminal();	
	startTime();	

	fftw_init_threads();
	fftw_plan_with_nthreads(FFTW_THREADS);
	fftw_make_planner_thread_safe();

	popLookUpTables();
	initOpenCV();	
	loadRefData();		
	normRefData();	
	fftw_execute(refPlan);	
	complxConjRef();		
	
	loadRangeData();
	
	for (int i = 0; i < THREADS; i++)
	{
		threadGroup.create_thread(boost::bind(&perThread, i));
	}
	
	threadGroup.join_all();		
	
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
		
		//std::cout << "(thread " << id << ") Processing Range Line: " << i << std::endl;
		
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
		
		/*FILE *pipe_gp = popen("gnuplot", "w");	
		fputs("set terminal postscript eps enhanced color font 'Helvetica,20' linewidth 2\n", pipe_gp);		
		std::stringstream ss;
		ss << "set title 'Thread " << id << "'\n";		
		fputs(ss.str().c_str(), pipe_gp);	
		fputs("set output 'output.eps' \n", pipe_gp);
		fputs("plot '-' using 1:2 with lines notitle\n", pipe_gp);
		for (int j = 0; j < PADRANGESIZE; j++) 
		{
			fprintf(pipe_gp, "%i %f\n", j, (realRangeBuffer[j + id*PADRANGESIZE]));
			//fprintf(pipe_gp, "%i %f\n", j, (sqrt(fftRangeBuffer[j  + id*(PADRANGESIZE/2 + 1)][0]*fftRangeBuffer[j  + id*(PADRANGESIZE/2 + 1)][0] + fftRangeBuffer[j  + id*(PADRANGESIZE/2 + 1)][1]*fftRangeBuffer[j  + id*(PADRANGESIZE/2 + 1)][1]))); 
			//fprintf(pipe_gp, "%i %f\n", j, (sqrt(hilbertBuffer[j  + id*(PADRANGESIZE)][0]*hilbertBuffer[j  + id*(PADRANGESIZE)][0] + hilbertBuffer[j  + id*(PADRANGESIZE)][1]*hilbertBuffer[j  + id*(PADRANGESIZE)][1]))); 
		}
		fputs("e\n", pipe_gp);
		pclose(pipe_gp);*/
		mutex.lock();
		updateWaterfall(i, &realRangeBuffer[id*PADRANGESIZE]);
		mutex.unlock();

		/*if (doppOn)
		{
			hilbertTransform();		//unnecessary cycles			
			popDopplerData(i); 		
			processDoppler(i);		
		}	*/				
	}	
	
	fftw_destroy_plan(rangePlan);
	fftw_destroy_plan(resultPlan);	
	
	std::cout << "(thread "<< id << ") Average Time per Line: "<< std::setprecision (2) << std::fixed << getTime()/RANGELINES * 1000000 << " us" << std::endl;
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
	printMsg("Complex Conjugate Reference");
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

	printMsg("Range Data Loaded");
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

	printMsg("Reference Data Loaded");	

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

	printMsg("Memory Free \n");
}



