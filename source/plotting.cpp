#include "plotting.hpp"

//globals
cv::Mat waterImage;

//final averaged doppler image
cv::Mat averageDopplerImage;
cv::Mat logDopplerImage;
cv::Mat processedDopplerImage;
cv::Mat normDopplerImage;

//vector to hold each threads current image
std::vector<cv::Mat> dopplerMatrix;
std::vector<cv::Mat> resizedDopplerMatrix;
		
cv::Mat processedImage;	
cv::Mat resizedWaterImage;
cv::Mat resizedDoppImage;
cv::Size waterSize(500, 500);
cv::Size doppSize(250, 500);

int waterfallColourMapSlider = 2;
int dopplerColourMapSlider = 2;
int	dopplerThresholdSlider = 0;
int n_plot_updates = 0;

const int dopplerThresholdMax = 255;
const int colourMapMax = 11;

void initPlots(void)
{	
	waterImage = cv::Mat::ones(RANGELINES, PADRANGESIZE, CV_64F);
	averageDopplerImage = cv::Mat(500, 250, CV_64F, cv::Scalar::all(0));
	
	dopplerMatrix.resize(THREADS);
	resizedDopplerMatrix.resize(THREADS);
	
	cv::namedWindow("Control Window", cv::WINDOW_NORMAL);
	cv::moveWindow("Control Window", 870, 100); 	
	
	cv::namedWindow("Range-Time-Intensity Plot");
	cv::moveWindow("Range-Time-Intensity Plot", 100, 100);					//trackbar is 54 units in height
	cv::createTrackbar( "RTI Colour Map", "Control Window", &waterfallColourMapSlider, colourMapMax);

	if (isDoppler == true)
	{
		cv::namedWindow("Range-Doppler Plot");
		cv::moveWindow("Range-Doppler Plot", 600, 100); 
		cv::createTrackbar("Threshold Value", "Control Window", &dopplerThresholdSlider, dopplerThresholdMax);
		cv::createTrackbar("Doppler Colour Map", "Control Window", &dopplerColourMapSlider, colourMapMax);
	}	
	
	//std::cout << "Initialized Plots" << std::endl;
}

void updateWaterfall(int rangeLine, double *imageValues)
{
	cv::Mat matchedRow = cv::Mat(1, PADRANGESIZE, CV_64F, imageValues);	
	cv::abs(matchedRow);		
	matchedRow.copyTo(waterImage(cv::Rect(0, rangeLine, matchedRow.cols, matchedRow.rows)));
}

void plotWaterfall(void)
{					
	processImage();
	cv::imshow("Range-Time-Intensity Plot", processedImage);	
	cv::waitKey(1);	
}


void processImage(void)
{
	cv::resize(waterImage, resizedWaterImage, waterSize);	
	cv::log(resizedWaterImage, resizedWaterImage);
	cv::normalize(resizedWaterImage, resizedWaterImage, 0.0, 1.0, cv::NORM_MINMAX);

	resizedWaterImage.convertTo(processedImage, CV_8U, 255);	
	
	cv::equalizeHist(processedImage, processedImage);

	cv::applyColorMap(processedImage, processedImage, waterfallColourMapSlider);	
	cv::transpose(processedImage, processedImage);
	cv::flip(processedImage, processedImage, 0);
}


void updateDoppler(int thread_id, double *imageValues)
{
	cv::Mat dopplerRow = cv::Mat(1, DOPPLERSIZE, CV_64F, imageValues);
	dopplerMatrix[thread_id].push_back(dopplerRow);
}

void plotDoppler(int thread_id)
{
	n_plot_updates++;
	cv::resize(dopplerMatrix[thread_id], resizedDopplerMatrix[thread_id], doppSize);	
	dopplerMatrix[thread_id].release();
	
	averageDopplerImage = averageDopplerImage + resizedDopplerMatrix[thread_id];		
			
	//cv::log(averageDopplerImage/n_plot_updates, logDopplerImage);
	cv::normalize(averageDopplerImage, normDopplerImage, 0.0, 1.0, cv::NORM_MINMAX);	
	
	normDopplerImage.convertTo(processedDopplerImage, CV_8U, 255);	
	
	cv::threshold(processedDopplerImage, processedDopplerImage, dopplerThresholdSlider, dopplerThresholdMax, 3);
	
	//cv::equalizeHist(processedDopplerImage, processedDopplerImage);	
		
	cv::applyColorMap(processedDopplerImage, processedDopplerImage, dopplerColourMapSlider);
	cv::flip(processedDopplerImage, processedDopplerImage, 0);

	cv::imshow("Range-Doppler Plot", processedDopplerImage);
	
	cv::waitKey(1);
	resizedDopplerMatrix[thread_id].release();	
	//processedDopplerImage.release();
}

void savePlots(void)
{
	processImage();	
	cv::imwrite("../results/rti.png", processedImage);
	if (isDoppler) cv::imwrite("../results/ard.png", processedDopplerImage);
}

void saveData(void)
{
	/*cv::FileStorage file("../results/waterfall_data.yml", cv::FileStorage::WRITE);	
	file << "Waterfall" << waterImage;	
	file.release();*/
		
	std::ofstream outputFile("../results/waterfall_data.csv");
    outputFile << format(waterImage, "CSV") << std::endl;
    outputFile.close();	
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
	
	
	
	//usefull to put in main...	
	/*FILE *pipe_gp = popen("gnuplot", "w");	
	fputs("set terminal postscript eps enhanced color font 'Helvetica,20' linewidth 2\n", pipe_gp);		
	std::stringstream ss;
	ss << "set title 'Thread " << id << "'\n";		
	fputs(ss.str().c_str(), pipe_gp);	
	fputs("set output 'output.eps' \n", pipe_gp);
	fputs("plot '-' using 1:2 with lines notitle\n", pipe_gp);
	for (int j = 0; j < PADRANGESIZE; j++) 
	{
		fprintf(pipe_gp, "%i %f\n", j, (realRangeBuffer[j + id*PADRANGESIZE]));
		//fprintf(pipe_gp, "%i %f\n", j, (sqrt(fftRangeBuffer[j  + id*(PADRANGESIZE/2 + 1)][0]*fftRangeBuffer[j  + id*(PADRANGESIZE/2 + 1)][0] + fftRangeBuffer[j  + id*(PADRANGESIZE/2 + 1)][1]*fftRangeBuffer[j  + id*(PADRANGESIZE/2 + 1)][1]))); 
		//fprintf(pipe_gp, "%i %f\n", j, (sqrt(hilbertBuffer[j  + id*(PADRANGESIZE)][0]*hilbertBuffer[j  + id*(PADRANGESIZE)][0] + hilbertBuffer[j  + id*(PADRANGESIZE)][1]*hilbertBuffer[j  + id*(PADRANGESIZE)][1]))); 
	}
	fputs("e\n", pipe_gp);
	pclose(pipe_gp);*/
}
