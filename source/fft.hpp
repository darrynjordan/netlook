#ifndef FFT_HPP
#define FFT_HPP

#include <fftw3.h>
#include "main.hpp"

void fftDopplerData(void);
void fftRefData(void);
void fftRangeData(void);
void ifftMatchedData(void);
void hilbertTransform(void);
void freePlanMemory(void);

#endif


