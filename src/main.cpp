#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <signal.h>
#include <cstdlib>
#include "codebook.h"

#include <cmath>
#include <algorithm>

using namespace cv;
using namespace std;
typedef Codebook::RunMode RunMode;

Codebook*   Codebook::codebooks;
RunMode     Codebook::mode;     // 0 playing, 1 training
int		    Codebook::cwCount = 0;
int         Codebook::width;
int         Codebook::height;
float	    Codebook::alpha = 0.5;
float	    Codebook::beta = 1.4;
float	    Codebook::eps = 20;
string      Codebook::fileName;
unsigned short Codebook::frameCount;
unsigned char* Codebook::outputFrameBytes;
bool        interruptSignal = false;
bool        morphOn = false;
int         seDefaultSize=1;
int         separation;
uint8_t*    tempArray;

void signalHandler(int signum){
    cout << "\nStopping..." << endl;
    interruptSignal = true;
}

void applyMorphology(Mat src, Mat dst, int seSize=seDefaultSize) {

    Mat se = getStructuringElement( MORPH_ELLIPSE, Size( 2*seSize + 1, 2*seSize+1 ), Point( seSize, seSize ) );
    morphologyEx(src, dst, MORPH_CLOSE, se);

}

int main(int argc, char* argv[])
{
    //Codebook::load("default.cbf");
    //delete Codebook::codebooks;
    //return 0;

    signal(SIGINT,signalHandler);

	if(argc < 3){
		cout << "Invalid input." << endl;
		return -1;
	}

	string type = argv[1];
	separation = atof((argv[2]));
	if(!separation){
		cout << "Enter a valid separation percentage." << endl;
		return -1;
	}
	int ai=3;
	while (ai<argc-1) {
        double temp = atof(argv[ai+1]);
        if ( !strcmp( argv[ai], "-a" ) ) {
            if(temp==0.0){
                cout << "Enter a valid alpha." << endl;
                return -1;
            }
            Codebook::alpha = temp;
        }
        else if ( !strcmp( argv[ai], "-b" ) ) {
            if(temp==0.0){
                cout << "Enter a valid beta." << endl;
                return -1;
            }
            Codebook::beta = temp;
        }
        else if ( !strcmp( argv[ai], "-e" ) ) {
            if(temp==0.0){
                cout << "Enter a valid epsilon." << endl;
                return -1;
            }
            Codebook::eps = temp;
        }
        else if ( !strcmp( argv[ai], "-se" ) ) {
            if(temp==0.0){
                cout << "Enter a se size." << endl;
                return -1;
            }
            seDefaultSize = temp;
        }
        else if ( !strcmp( argv[ai], "-m" ) ) {
            morphOn = true;
        }
        ai++;
	}
	cout    << "Alpha: " << Codebook::alpha
            << ". Beta: " << Codebook::beta
            << ". Epsilon: " << Codebook::eps << "." << endl;

	bool train = false;
	if(type == "t"){
		train = true;
	} else if (type == "p"){
		train = false;
	} else {
		cout << "Unknown parameter; use either t or p." << endl;
		return -1;
	}

	/*VideoCapture cap(videoFile); // open the video file for reading

	if ( !cap.isOpened() ) { // if not success, exit program
		cout << "Cannot open the video file" << endl;
		return -1;
	}

    Codebook::width = (int) cap.get(CV_CAP_PROP_FRAME_WIDTH);
    Codebook::height = (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    Codebook::fileName = "default.cbf";
    int frameCount = (int) cap.get(CV_CAP_PROP_FRAME_COUNT);
    */

    tempArray = new uint8_t[ 288*384 *3];
	Mat frame;
    stringstream ss;
    ss << "video/PetsD2TeC1_" << setw(5) << setfill('0') << 0 << ".jpg";
    frame = imread(ss.str().c_str(), CV_LOAD_IMAGE_COLOR);

    Codebook::width = frame.cols;
    Codebook::height = frame.rows;
    Codebook::fileName = "default.cbf";
    int frameCount = 2824;
    separation = frameCount*separation/100;
    Codebook::frameCount = frameCount;

	//cap.set(CV_CAP_PROP_POS_MSEC, 300); //start the video at 300ms

	if (train) {
        time_t start = time(0);
        Codebook::initCodebooks(Codebook::TRAIN);

        int frameNumber = 0;
		while(!interruptSignal && frameNumber<separation) {

			stringstream ss;
            ss << "video/PetsD2TeC1_" << setw(5) << setfill('0') << frameNumber << ".jpg";
            frame = imread(ss.str().c_str(), CV_LOAD_IMAGE_COLOR);
			frameNumber++;
			/*Mat testFrame(Codebook::height, Codebook::width, CV_8UC3, frame.data);
            imshow("test2",testFrame);
            waitKey(1);*/
			Codebook::processFrame(frame.data, frameNumber);

			int perc = (int) (100*frameNumber/separation);
			cout << "\rProgress: " << perc << "%. Total codeword count = " << Codebook::cwCount << flush;

		}
        Codebook::applyMNRL();
		cout << "\n";
		if(!interruptSignal)
            Codebook::save();

		time_t difference = time(0) - start ;
		cout << "Average codeword count: " << Codebook::cwCount/(Codebook::width * Codebook::height) << endl;
		cout << "Time passed: " << difference << "sec" << endl;

	}
	else {

        Codebook::initCodebooks(Codebook::PLAY);

		/*
		double fps = 25;//cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
		double period_us = 1000000/fps;
		int waitKeyDur = 1;//(int) (period_us/1000-20);
		cout << "Frame per seconds: " << fps << endl;
		cout << "Period in usecs: " << period_us << endl;

		#ifdef _WIN32
        #else
        chrono::steady_clock::time_point next = chrono::steady_clock::now();
        period_us = 10;
        #endif
        */

		namedWindow("Background Substraction", CV_WINDOW_AUTOSIZE);
        int frameNumber = separation;

		while(frameNumber<frameCount && !interruptSignal) {

			stringstream ss;
            ss << "video/PetsD2TeC1_" << setw(5) << setfill('0') << frameNumber << ".jpg";
            frame = imread(ss.str().c_str(), CV_LOAD_IMAGE_COLOR);
			frameNumber++;

            Codebook::processFrame(frame.data, 0);
            Mat foreground(frame.size(), CV_8U, Codebook::outputFrameBytes);
            if (morphOn)
                applyMorphology(foreground, foreground);

			imshow("Background Substraction", foreground);
			waitKey(1);
			/*
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
            */
		}

	}
    //cap.release();
    Codebook::endCodebooks();
    if (interruptSignal) {
        cout << "\nProcess terminated." << endl;
        //cin.get();
    }

            delete [] tempArray;
	return 0;
}
