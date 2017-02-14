#include "general.hpp"

double 	tStart, tEnd;

void initTerminal(void)
{
	system("clear\n");
	printf("NetRAD RTI Processor\n");
	printf("--------------------\n");
	printf("\n");		

	if (isDoppler)
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
		
	printf("\n");
	printf("========================================\n");
	printf("Number of Range Lines:\t%i\n", RANGELINES);
	printf("Samples per Range Line:\t%i\n", RANGESIZE);
	printf("Zero Padded Size:\t%i\n", PADRANGESIZE);
	printf("Threads for FFTW:\t%i\n", FFTW_THREADS);
	printf("Concurrent Threads:\t%i\n", THREADS);
	printf("Range Lines per Thread:\t%i\n", RANGELINESPERTHREAD);
	printf("========================================\n");
	printf("\n");

}

float getTime(void)
{
	struct timeval time;
    gettimeofday(&time,NULL);
	tEnd = (double)time.tv_sec + (double)time.tv_usec * .000001;
	return (float)(tEnd - tStart);
}

void startTime(void)
{
	std::cout << "Starting Timer...\n" << std::endl;
	struct timeval time;
    gettimeofday(&time,NULL);
	tStart = (double)time.tv_sec + (double)time.tv_usec * .000001;	
}

void endTime(void)
{
	std::cout << "Completed Processing in " << std::setprecision (2) << std::fixed << getTime() << "s\n" << std::endl;	
}

void printMsg(std::string msg)
{
	std::cout << std::setprecision (2) << std::fixed << getTime() << "s:\t" << msg << std::endl;	
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

	for (int i = 0; i < DOPPLERSIZE; i++)
	{
		doppWindow[i] = getWindowFactor(i, DOPPLERSIZE, HANNING);
	}
	std::cout << "Calculated Window Values" << std::endl;
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


