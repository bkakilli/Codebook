#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace cv;
using namespace std;

class Codebook {
	
	public:
	
	typedef struct{
		double rgb[3];
		double minI;
		double maxI;
		int f;
		int l;
		int p;
		int q;
	} Codeword;
	
	
	vector<Codeword *> cwlist;
	~Codebook();
	
	static int cwCount;
	static double eps;
	static double alpha;
	static double beta; 
	static void processFrame(Mat frame, int t, Codebook *codebooks);
	static bool isMatched( Codeword *cw, double rgb[3], double I  );
	
};