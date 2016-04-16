#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <signal.h>
#include <cstdlib>
#include "codebook.h"

using namespace cv;
using namespace std;
typedef Codebook::RunMode RunMode;

Codebook*   Codebook::codebooks;
RunMode     Codebook::mode;     // 0 playing, 1 training
int		    Codebook::cwCount = 0;
int         Codebook::width;
int         Codebook::height;
float	    Codebook::alpha = 0.5;
float	    Codebook::beta = 1.2;
float	    Codebook::eps = 1;
string      Codebook::fileName;
unsigned char* Codebook::outputFrameBytes;
bool        interruptSignal = false;

void signalHandler(int signum){
    interruptSignal = true;
}

int main(int argc, char* argv[])
{
    //Codebook::load("default.cbf");
    //delete Codebook::codebooks;
    //return 0;

    signal(SIGINT,signalHandler);

	if(argc < 2){
		cout << "Invalid input." << endl;
		return -1;
	}

	string videoFile = argv[1];
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

	VideoCapture cap(videoFile); // open the video file for reading

	if ( !cap.isOpened() ) { // if not success, exit program
		cout << "Cannot open the video file" << endl;
		return -1;
	}

    Codebook::width = (int) cap.get(CV_CAP_PROP_FRAME_WIDTH);
    Codebook::height = (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    Codebook::fileName = "default.cbf";
    int frameCount = (int) cap.get(CV_CAP_PROP_FRAME_COUNT);

	//cap.set(CV_CAP_PROP_POS_MSEC, 300); //start the video at 300ms
	Mat frame;

	if (train) {
        time_t start = time(0);
        Codebook::initCodebooks(Codebook::TRAIN);

		int frameNumber = 0;

		while(cap.read(frame) && !interruptSignal && frameNumber<200) {

			frameNumber++;
			Codebook::processFrame(frame, frameNumber);
			int perc = (int) (100*frameNumber/frameCount);
			cout << "\rProgress: " << perc << "%. Total codeword count = " << Codebook::cwCount << flush;

		}
		cout << "\n";
		if(!interruptSignal)
            Codebook::save();

		time_t difference = time(0) - start ;
		cout << "Time passed: " << difference << endl;

	}
	else {

        Codebook::initCodebooks(Codebook::PLAY);

		double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
		double period_us = 1000000/fps;
		int waitKeyDur = 1;//(int) (period_us/1000-20);
		cout << "Frame per seconds: " << fps << endl;
		cout << "Period in usecs: " << period_us << endl;

		#ifdef _WIN32
        #else
        chrono::steady_clock::time_point next = chrono::steady_clock::now();
        period_us = 10;
        #endif

		namedWindow("Background Substraction", CV_WINDOW_AUTOSIZE);

		while(cap.read(frame) && !interruptSignal) {

            Codebook::processFrame(frame, 0);
            Mat foreground(frame.size(), CV_8U);
            foreground.data = Codebook::outputFrameBytes;
			imshow("Background Substraction", foreground);
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
    cap.release();
    Codebook::endCodebooks();
    if (interruptSignal)
        cout << "\n\nProcess terminated." << endl;

	return 0;
}
