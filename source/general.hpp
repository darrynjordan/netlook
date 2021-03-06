#ifndef GENERAL_HPP
#define GENERAL_HPP

#include "main.hpp"
#include <iomanip> 
#include <time.h>
#include <sys/time.h>

enum window {HANNING, HAMMING, BLACKMAN};

void initTerminal(void);
void help(void);
void printMsg(std::string msg);
void primeSolver(void);
void popLookUpTables(void);
void timingSummary(void);
float getWindowFactor(int sample, int numSamples, int window);

#endif
