#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>

using namespace cv;
using namespace std;

typedef struct{
    unsigned short w;
    unsigned short h;
    float rgb[3];
    float minI;
    float maxI;
    int f;
    int l;
    int p;
    int q;
} Codeword;

typedef struct{
    unsigned short width;
    unsigned short height;
    float eps;
    float alpha;
    float beta;
    int cwCount;
} CBFHeader;

class Codebook {

	public:
    enum RunMode {PLAY=0, TRAIN};
	vector<Codeword *> cwlist;
	~Codebook();

	static Codebook *codebooks;
	static RunMode mode;
	static int cwCount;
	static int width;
	static int height;
	static float eps;
	static float alpha;
	static float beta;
	static string fileName;
	static unsigned char *outputFrameBytes;
	static void initCodebooks(RunMode mode);
	static void endCodebooks();
	static void processFrame(Mat frame, int t);
	static bool isMatched( Codeword *cw, float rgb[3], float I  );
	static void wrapCodebooks();
	static bool fileExists(string fileName);
	static void save();
	static void load();
	static void load(string fn);

};
