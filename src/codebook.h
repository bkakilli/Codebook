#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>

using namespace cv;
using namespace std;

class Codebook {

	public:
    enum RunMode {PLAY=0, TRAIN};

	typedef struct{
		float rgb[3];
		float minI;
		float maxI;
		int f;
		int l;
		int p;
		int q;
		unsigned short w;
		unsigned short h;
	} Codeword;


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
	static void processFrame(Mat frame, int t);
	static bool isMatched( Codeword *cw, float rgb[3], float I  );
	static void save(string fileName);
	static void load(string fileName);

};
