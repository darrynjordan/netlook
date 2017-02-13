#include "plotting.hpp"

//globals
cv::Mat waterImage;
cv::Mat doppImage;
cv::Mat resizedWaterImage, resizedDoppImage;
cv::Size waterSize(500, 500);
cv::Size doppSize(250, 500);

int waterfallColourMapSlider = 0;
int dopplerColourMapSlider = 0;

const int dopplerThresholdMax = 255;
const int colourMapMax = 11;

void initOpenCV(void)
{	
	cv::namedWindow("Control Window", cv::WINDOW_NORMAL);
	cv::moveWindow("Control Window", 870, 100); 	
	
	cv::namedWindow("Waterfall Plot");
	cv::moveWindow("Waterfall Plot", 100, 100);					//trackbar is 54 units in height
	cv::createTrackbar( "Waterfall Colour Map", "Control Window", &waterfallColourMapSlider, colourMapMax);

	if (doppOn == true)
	{
		cv::namedWindow("Doppler Plot");
		cv::moveWindow("Doppler Plot", 600, 100); 
		cv::createTrackbar( "Threshold Value", "Control Window", &dopplerThresholdSlider, dopplerThresholdMax);
		cv::createTrackbar( "Doppler Colour Map", "Control Window", &dopplerColourMapSlider, colourMapMax);
	}

	printMsg("Initialized OpenCV");
}

void updateWaterfall(int rangeLine, double *imageValues)
{
	cv::Mat matchedRow = cv::Mat(1, PADRANGESIZE, CV_64F, imageValues);
	
	cv::abs(matchedRow);	
	
	waterImage.push_back(matchedRow);
	
	//plotWaterfall();
	//cv::waitKey(0);	

	if (((rangeLine%(UPDATELINE-1) == 0) || rangeLine == (RANGELINES-1)) && rangeLine != 0)
	{
		plotWaterfall();
		cv::waitKey(0);
	}
}

void plotWaterfall(void)
{					
	cv::resize(waterImage, resizedWaterImage, waterSize);		
	cv::log(resizedWaterImage, resizedWaterImage);
	cv::normalize(resizedWaterImage, resizedWaterImage, 0.0, 1.0, cv::NORM_MINMAX);

	cv::Mat processedImage;	
	resizedWaterImage.convertTo(processedImage, CV_8U, 255);	
	
	cv::equalizeHist(processedImage, processedImage);

	cv::applyColorMap(processedImage, processedImage, waterfallColourMapSlider);	
	cv::transpose(processedImage, processedImage);
	cv::flip(processedImage, processedImage, 0);

	cv::imshow("Waterfall Plot", processedImage);
	//cv::imwrite("waterfall_plot.png", resizedWaterImage);
	cv::waitKey(1);	
	processedImage.release();
	//waterImage.release();
}

void updateDoppler(uint8_t  *imageValues)
{
	cv::Mat row = cv::Mat(1, DOPPLERSIZE, CV_8U, imageValues);
	doppImage.push_back(row);
}

void plotDoppler(void)
{
	cv::resize(doppImage, resizedDoppImage, doppSize);	
	doppImage.release();
	cv::applyColorMap(resizedDoppImage, resizedDoppImage, dopplerColourMapSlider);
	cv::flip(resizedDoppImage, resizedDoppImage, 0);

	cv::imshow("Doppler Plot", resizedDoppImage);
	//cv::imwrite("doppler_plot.png", resizedDoppImage);
	cv::waitKey(1);
	resizedDoppImage.release();	
	doppImage.release();
}

void GNUplot(void)
{
/*	FILE *pipe_gp = popen("gnuplot", "w");	

	fputs("set terminal postscript eps enhanced color font 'Helvetica,20' linewidth 2\n", pipe_gp);
	fputs("set title 'Doppler Processing Time per Range Line'\n", pipe_gp);	
	//fputs("set yrange [0:0.2] \n", pipe_gp);
	fputs("set xrange[1:64] \n", pipe_gp);
	//fputs("set bmargin at screen 0.13 \n", pipe_gp);
	//fputs("set lmargin at screen 0.13 \n", pipe_gp);
	//fputs("set xtics ('0' 0, '10' 205, '20' 410, '30' 615, '40' 820, '50' 1024) \n", pipe_gp);
	//fputs("set xtics ('-500' 0, '-376' 32, '-252' 64, '-128' 96, '0' 128, '128' 160, '252' 192, '376' 224, '500' 255) \n", pipe_gp);
	fputs("set xlabel 'Range Line' \n", pipe_gp);
	fputs("set ylabel 'Time [s]' \n", pipe_gp);
	fputs("set output 'output.eps' \n", pipe_gp);
	fputs("plot '-' using 1:2 with lines notitle\n", pipe_gp);


	for (int i = 0; i < 64; i++) 
		fprintf(pipe_gp, "%i %f\n", i, (realRangeBuffer[i]));
		//fprintf(pipe_gp, "%i %f\n", i, (sqrt(dopplerBuffer[i][0]*dopplerBuffer[i][0] + dopplerBuffer[i][1]*dopplerBuffer[i][1]))); 

	fputs("e\n", pipe_gp);
	pclose(pipe_gp);*/
}
