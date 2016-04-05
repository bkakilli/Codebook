#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
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

	if(argc < 2){
		cout << "Invalid input." << endl;
		return -1;
	}

	string fileName = argv[1];
	string type = argv[2];

	bool train = false;
	if(type == "t"){
		train = true;
	} else if (type == "p"){
		train = false;
	} else {
		cout << "Unknown parameter; use either t or p." << endl;
		return -1;
	}

	VideoCapture cap(fileName); // open the video file for reading

	if ( !cap.isOpened() ) { // if not success, exit program
		cout << "Cannot open the video file" << endl;
		return -1;
	}

	//cap.set(CV_CAP_PROP_POS_MSEC, 300); //start the video at 300ms
	Mat frame;

	if (train) {

		int width = (int) cap.get(CV_CAP_PROP_FRAME_WIDTH);
		int height = (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT);
		int frameCount = (int) cap.get(CV_CAP_PROP_FRAME_COUNT);

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

		double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
		double period_us = 1000000/fps;
		int waitKeyDur = (int) (period_us/1000-5);
		cout << "Frame per seconds: " << fps << endl;
		cout << "Period in usecs: " << period_us << endl;

		#ifdef _WIN32
        #else
        chrono::steady_clock::time_point next = chrono::steady_clock::now();
        period_us = 10;
        #endif

		namedWindow("Background Substraction",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
		// Read until stream ends
		while(cap.read(frame)) {

			imshow("Background Substraction", frame);
			if(waitKey(waitKeyDur) == 27) {
				cout << "esc key is pressed by user" << endl;
				break;
			}

            #ifdef _WIN32
            #else
			// Accurate timing
			this_thread::sleep_until(next);
			next = next + chrono::microseconds((int) period_us - 10000);
            #endif

		}
	}
	return 0;

}
