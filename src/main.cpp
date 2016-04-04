#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "codebook.h"

using namespace cv;
using namespace std;

int		Codebook::cwCount = 0;
double	Codebook::alpha = 0.5;
double	Codebook::beta = 1.2;
double	Codebook::eps = 5;

int main(int argc, char* argv[])
{
	
	VideoCapture cap("sample.avi"); // open the video file for reading

	if ( !cap.isOpened() ) { // if not success, exit program
		cout << "Cannot open the video file" << endl;
		return -1;
	}

	//cap.set(CV_CAP_PROP_POS_MSEC, 300); //start the video at 300ms

	double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
	double period_us = 1000000/fps;
	cout << "Frame per seconds: " << fps << endl;
	cout << "Period in usecs: " << period_us << endl;

	//namedWindow("MyVideo",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"

	Mat frame;
	chrono::steady_clock::time_point next = chrono::steady_clock::now();
	int count = 0;
	
	int width = (int) cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	int frameCount = (int) cap.get(CV_CAP_PROP_FRAME_COUNT);
	
	bool train = true;
	if (train) {
		
		// Create codebooks
		Codebook *codebooks = new Codebook[ width*height ];

		int frameNumber = 0;
		// Read until stream ends
		while(cap.read(frame)) {
			
			frameNumber++;
			Codebook::processFrame(frame, frameNumber, codebooks);
			int perc = (int) (100*frameNumber/frameCount);
			cout << "\rProgress: " << perc << "%. Total codeword count = " << Codebook::cwCount << flush;

		}
		cout << "\n";

		delete [] codebooks;
		
	}
	else {
		
		// Read until stream ends
		while(cap.read(frame)) {
			
			imshow("MyVideo", frame);
			if(waitKey(10) == 27) {
				cout << "esc key is pressed by user" << endl; 
				break; 
			}

			// Accurate timing
			this_thread::sleep_until(next); 
			next = next + chrono::microseconds((int) period_us - 10000);

		}
	}
	return 0;

}
