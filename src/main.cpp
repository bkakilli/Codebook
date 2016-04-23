#include "opencv2/opencv.hpp"
#include <iomanip>
#include <string>
#include <vector>
#include <signal.h>
#include <dirent.h>
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
float	    Codebook::beta = 1.4;
float	    Codebook::eps = 20;
string      Codebook::fileName = "";
unsigned short Codebook::frameCount;
unsigned char* Codebook::outputFrameBytes;
bool        endProgram = false;
bool        morphOn = false;
int         seDefaultSize=1;
int         trainPerc = 50;
int         firstFrame;
int         lastFrame;
uint8_t*    tempArray;
vector<string> fileList;
int         frameCount;
int         frameNumber = 0;
int         sourceType;
int         morphType = 3;
VideoCapture cap;
bool train = false;

void signalHandler(int signum){
    cout << "\nStopping..." << endl;
    endProgram = true;
}

void applyMorphology(Mat src, Mat dst, int seSize=seDefaultSize) {
    Mat se = getStructuringElement( MORPH_ELLIPSE, Size( 2*seSize + 1, 2*seSize+1 ), Point( seSize, seSize ) );
    morphologyEx(src, dst, morphType, se);
}

bool initSource(string source) {

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir (source.c_str())) != NULL) {   // Folder
        sourceType = 1;
        if (source.at(source.size()-1) != '/' && source.at(source.size()-1) != '\\')
            source = source + "/";
        frameCount=0;
        while ((ent = readdir (dir)) != NULL ) {
            fileList.push_back(source+ent->d_name);
            frameCount++;
        }
        closedir (dir);
        fileList.erase(fileList.begin(), fileList.begin()+2); // Remove . and ..
        frameCount -= 2;

        Mat frame = imread(fileList.at(0), CV_LOAD_IMAGE_COLOR);
        Codebook::width = frame.cols;
        Codebook::height = frame.rows;

    }
    else {              // Video or device
        if(!cap.open(source))
            return false;
        sourceType = 2;
        frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
        Codebook::width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        Codebook::height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    }

    Codebook::frameCount = frameCount;

    if(train) {
        firstFrame = 0;
        lastFrame = frameCount*trainPerc/100;
    }
    else {
        firstFrame = frameCount*trainPerc/100;
        lastFrame = frameCount;
    }

    cap.set(CV_CAP_PROP_POS_FRAMES, firstFrame);
    frameNumber = firstFrame;

    return true;
}

bool getNext(Mat& frame) {

    try {
        if ( frameNumber<lastFrame ){
            if (sourceType==1 )
                frame = imread(fileList.at(frameNumber).c_str(), CV_LOAD_IMAGE_COLOR);
            else {
                if(!cap.read(frame))
                    return false;
            }
            frameNumber++;

            return true;
        }
    } catch (int e) {
        cout << "Error!" << endl;
        endProgram = true;
    }
    return false;
}

int main(int argc, char* argv[])
{
    signal(SIGINT,signalHandler);

	if(argc < 3){
		cout << "Invalid input." << endl;
		return -1;
	}

	train = false;
	if( !strcmp(argv[2], "t") ){
		train = true;
	} else if( !strcmp(argv[2], "p") ){
		train = false;
	} else {
		cout << "Specify the mode (t for training, p for playing)." << endl;
		return -3;
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
                cout << "Error: Enter a valid epsilon." << endl;
                return -1;
            }
            Codebook::eps = temp;
        }
        else if ( !strcmp( argv[ai], "-se" ) ) {
            if(temp==0.0){
                cout << "Error: Enter a valid structuring element size." << endl;
                return -1;
            }
            seDefaultSize = temp;
        }
        else if ( !strcmp( argv[ai], "-s" ) ) {
            trainPerc = temp;
        }
        else if ( !strcmp( argv[ai], "-m" ) ) {
            if(temp==0.0){
                cout << "Error: Enter a valid morphology type. (1,2,3,4)" << endl;
                return -1;
            }
            morphType = temp-1;
            morphOn = true;
        }
        else if ( !strcmp( argv[ai], "-f" ) ) {
            Codebook::fileName = argv[ai+1];
        }
        ai++;
	}
	if (train)
        cout    << "Alpha: " << Codebook::alpha << ", "
                << "Beta: " << Codebook::beta << ", "
                << "Epsilon: " << Codebook::eps << endl
                << "Destination file: " << Codebook::fileName << endl;

    if (!initSource(argv[1])) {
        cout << "Invalid source folder or video file" << endl;
        return -2;
    }
    //endProgram = true;

    Mat frame;
	if (train) {
        time_t start = time(0);

        if (!Codebook::initCodebooks(Codebook::TRAIN))
            endProgram = true;

        int t=0;
		while( getNext(frame) && !endProgram) {

			t++;
			Codebook::processFrame(frame.data, t);

			int perc = (int) (100*t/(lastFrame-firstFrame));
			cout << "\rProgress: " << perc << "%. Total codeword count = " << Codebook::cwCount << flush;

		}
		cout << "\n";
		if(!endProgram) {
            Codebook::applyMNRL();
            Codebook::save();
		}

		time_t difference = time(0) - start ;
		cout << "Average codeword count: " << Codebook::cwCount/(Codebook::width * Codebook::height) << endl;
		cout << "Time passed: " << difference << "sec" << endl;

	}
	else {

        if (!Codebook::initCodebooks(Codebook::PLAY))
            endProgram = true;

        if(getNext(frame)) {
            if(frame.rows != Codebook::height || frame.cols != Codebook::width) {
                cout << "Error: Given video and codebook file do not match." << endl;
                endProgram = true;
            }
        }

		namedWindow("Background Substraction", CV_WINDOW_AUTOSIZE);

		while(getNext(frame) && !endProgram) {

            Codebook::processFrame(frame.data, 0);
            Mat foreground(frame.size(), CV_8U, Codebook::outputFrameBytes);
            if (morphOn)
                applyMorphology(foreground, foreground);

			imshow("Background Substraction", foreground);
			if(waitKey(1) == 'c'){
                stringstream ss;
                ss << "snapshots/" << Codebook::fileName << "_cap" << frameNumber << ".jpg";
                imwrite(ss.str(), foreground);
			}
		}
	}

    Codebook::clearCodebooks();
    if (endProgram) {
        cout << "\nProcess terminated." << endl;
        //cin.get();
    }

    cap.release();
	return 0;
}
