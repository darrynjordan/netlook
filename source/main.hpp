#ifndef MAIN_HPP
#define MAIN_HPP

//libraries
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <boost/thread.hpp>
#include <time.h>
#include <sys/time.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "plotting.hpp"
#include "fft.hpp"
#include "general.hpp"
#include "data.hpp"

//parameters

#define	ADCOFFSET		8200

#define ERROR 			1000
#define MAXPOW 			20

//functions
void allocateMemory(void);
void loadRangeData(void);
void loadRefData(void);
void popRangeBuffer(int rangeLine, double* realRangeBuffer);
void popDopplerData(int rangeLine);
void popDopplerBuffer(int dopplerLine);
void complxConjRef(void);
void getParameters(void);
void perThread(int id);
void freeMemory(void);
void processingLoop(int repetitions);
void postProcessMatched(void);
void postProcessDoppler(void);
void normRefData(void);
void processDoppler(int rangeLine);
float getNormFactor(void);
uint16_t getRangeOffset(void);

void restartTimer(void);
double getTimeElapsed(void);

//globals
extern uint16_t 	*realDataBuffer;
extern double   	*realRefBuffer;
extern uint8_t  	*dopplerImageBuffer;
extern fftw_complex *fftRangeBuffer;
extern fftw_complex *fftRefBuffer; 
extern fftw_complex *hilbertBuffer;
extern fftw_complex *dopplerBuffer;
extern fftw_complex *dopplerData;
extern float		*refWindow;
extern float		*doppWindow;

extern bool isDoppler;
extern bool isSuggestions;
extern int dopplerThresholdSlider;

extern int DATASETID;
extern int REFSIZE;
extern int PADRANGESIZE;
extern int RANGELINES;
extern int RANGESIZE;
extern int DOPPLERSIZE;
extern int UPDATELINE;
extern int FFTW_THREADS;
extern int THREADS;
extern int RANGELINESPERTHREAD;
extern int REPETITIONS;
extern int PLANNER_FLAG;

#endif
