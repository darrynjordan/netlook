#ifndef PLOTTING_HPP
#define PLOTTING_HPP

//Libraries
#include <opencv2/opencv.hpp>
#include "main.hpp"
#include <fstream>

//Functions
void plotWaterfall(void);
void plotDoppler(void);
void updateWaterfall(int rangeLine, double *imageValues);
void updateDoppler(uint8_t  *imageValues);
void initPlots(void);
void processPlot(void);
void initMat(void);
void savePlots(void);
void saveData(void);
void GNUplot(void);

extern int waterfallColourMapSlider;

#endif
