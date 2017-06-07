#include "general.hpp"

void initTerminal(void)
{
	system("clear\n");
	printf("NetRAD RTI Processor\n");
	
	/*if (isDoppler)
		printf("Doppler Processing:\t\tON\n");
	else
		printf("Doppler Processing:\t\tOFF\n");

	if (isSuggestions)
	{
		printf("\n");		
		printf("Suggested Zero Padding Sizes:\n");
		primeSolver();
	}
	else
		printf("Zero Padding Size Suggestions:\tOFF\n");
		
	printf("\n");*/
	printf("========================================\n");
	printf("Dataset ID:\t\t%i\n", DATASETID);
	printf("Number of Range Lines:\t%i\n", RANGELINES);
	printf("Samples per Range Line:\t%i\n", RANGESIZE);
	printf("Zero Padded Size:\t%i\n", PADRANGESIZE);
	printf("FFTW Threads:\t\t%i\n", FFTW_THREADS);
	printf("Course Threads:\t\t%i\n", THREADS);
	printf("Range Lines per Thread:\t%i\n", RANGELINESPERTHREAD);
	printf("Processing Repetitions:\t%i\n", REPETITIONS);
	
	switch (PLANNER_FLAG)
	{
		case 0:
			printf("Planner Flag: \t\tFFTW_MEASURE\n");
			break;
		case 64:
			printf("Planner Flag: \t\tFFTW_ESTIMATE\n");
			break;		
		default:
			printf("ERROR: Unknown Planner Flag!\n\n");
			exit(EXIT_FAILURE);
	}	
	
	printf("========================================\n");
	printf("\n");

}


void help(void)
{
	system("clear\n");
	printf("NetRAD RTI Processor: Help\n");
	printf("========================================================\n");
	printf(" -h: display this help screen\n");
	printf(" -d: select the dataset id \t(0 - 9)\n");
	printf(" -l: number of range lines \t(1 - 130000)\n");
	printf(" -z: bins to zero pad up to \t(2048 - 10280)\n");
	printf(" -t: number of course threads\n");
	printf(" -f: number of threads per fft\n");
	printf(" -p: planner flag for fftw \t(0 or 64)\n");
	printf(" -r: number of repetitions\n");
	printf(" -v: turn openCV visualisation on\n");
	printf(" -c: initial colour map \t(1 - 11)\n");	
	printf(" -u: range lines per update of visualisation\n");
	
	printf("\n");
	printf("Predicted maximum number of threads for your system: %i\n", boost::thread::hardware_concurrency());
	printf("========================================================\n");
	printf("\n");
	exit(EXIT_SUCCESS);	
}


void timingSummary(void)
{
	std::ofstream timing_file;
	timing_file.open("timing.csv", std::ofstream::out | std::ofstream::app);
	timing_file << DATASETID << ", ";
	timing_file << RANGELINES << ", ";
	timing_file << PADRANGESIZE << ", ";
	timing_file << PLANNER_FLAG << ", ";
	timing_file << FFTW_THREADS << ", ";
	timing_file << THREADS << ", ";
	timing_file << RANGELINESPERTHREAD << ", ";
	timing_file << min_time << ", ";
	timing_file << max_time << ", ";
	timing_file << avg_time << " \n";
	timing_file.close();
}


void printMsg(std::string msg)
{
	std::cout << std::setprecision (2) << std::fixed << getTimeElapsed() << "s:\t" << msg << std::endl;	
}

float getWindowFactor(int sample, int numSamples, int window)
{
	switch(window)
	{
		case HANNING  : return 0.50 - 0.50*cos((2*3.14*sample)/(numSamples - 1)); break;
		case HAMMING  : return 0.54 + 0.46*cos((2*3.14*sample)/(numSamples - 1)); break;
	}	
}

void  popLookUpTables(void)
{
	for (int i = 0; i < REFSIZE; i++)
	{
		refWindow[i] = getWindowFactor(i, REFSIZE, HANNING);
	}
	
	for (int i = 0; i < RANGESIZE; i++)
	{
		rangeWindow[i] = getWindowFactor(i, RANGESIZE, HANNING);
	}

	for (int i = 0; i < DOPPLERSIZE; i++)
	{
		doppWindow[i] = getWindowFactor(i, DOPPLERSIZE, HANNING);
	}
	//std::cout << "Calculated Window Values" << std::endl;
}

void primeSolver(void)
{
	int avdCirConv = RANGESIZE + REFSIZE - 1;
	int soln = 0;	

	for (int i = 0; i < MAXPOW; i++)
	{
		for (int j = 0; j < MAXPOW; j++)
		{
			soln = pow(2, i) * pow(3, j);

			if ((soln >= avdCirConv) and (soln < (avdCirConv + ERROR)))
				std::cout << "2^" << i << "\t* 3^" << j << "\t= " << soln << std::endl;
			
		}
	}
}


