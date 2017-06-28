#ifndef PLOTTING_HPP
#define PLOTTING_HPP

//Libraries
#include <opencv2/opencv.hpp>
#include "main.hpp"
#include <fstream>

//Functions
void plotWaterfall(void);
void plotDoppler(int thread_id);
void updateWaterfall(int rangeLine, double *imageValues);
void updateDoppler(int thread_id, double *imageValues);
void initPlots(void);
void initMats(void);
void processImage(void);
void savePlots(void);
void saveData(void);
void GNUplot(void);
void saveData(void);

extern int waterfallColourMapSlider;

#endif
