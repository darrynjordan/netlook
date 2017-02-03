#ifndef GENERAL_HPP
#define GENERAL_HPP

#include "main.hpp"
#include <iomanip> 
#include <time.h>
#include <sys/time.h>

enum window {HANNING, HAMMING, BLACKMAN};

void  initTerminal(void);
void  startTime(void);
float getTime(void);
void  printMsg(std::string msg);
float getWindowFactor(int sample, int numSamples, int window);
void  primeSolver(void);
void  popLookUpTables(void);

#endif
