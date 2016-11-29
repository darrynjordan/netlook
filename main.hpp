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

#include <iostream>
#include <sstream>
#include <fstream>

#include "utilities/plotting.hpp"
#include "utilities/fft.hpp"
#include "utilities/general.hpp"
#include "utilities/data.hpp"

//parameters

#define	ADCOFFSET		8200

#define ERROR 			1000
#define MAXPOW 			20

//functions
void loadRangeData(void);
void loadRefData(void);
void popRangeBuffer(int rangeLine);
void popDopplerData(int rangeLine);
void popDopplerBuffer(int dopplerLine);
void complxConjRef(void);
void complxMulti(void);
void matchedFilter(void);
void freeMem(void);

void postProcessMatched(void);
void postProcessDoppler(void);

void normRefData(void);
void processDoppler(int rangeLine);

float getNormFactor(void);
uint16_t getRangeOffset(void);

//globals
extern uint16_t 	*realDataBuffer;
extern double   	*realRangeBuffer;
extern double   	*realRefBuffer;
extern uint8_t  	*matchedImageBuffer;
extern uint8_t  	*dopplerImageBuffer;
extern fftw_complex *fftRangeBuffer;
extern fftw_complex *fftRefBuffer; 
extern fftw_complex *hilbertBuffer;
extern fftw_complex *dopplerBuffer;
extern fftw_complex *dopplerData;

extern float		*refWindow;
extern float		*doppWindow;

extern clock_t 		tStart, tEnd;
extern bool 		doppOn;
extern bool 		suggestOn;
extern int			dopplerThresholdSlider;

extern int 			REFSIZE;
extern int 			PADRANGESIZE;
extern int 			RANGELINES;
extern int 			RANGESIZE;
extern int 			DOPPLERSIZE;
extern int 			UPDATELINE;


extern std::vector<float> timePlot;

#endif
