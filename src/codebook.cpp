#include "codebook.h"

Codebook::~Codebook(){
	for(int i=0; i<cwlist.size(); i++)
		delete cwlist.at(i);
}

void Codebook::processFrame(Mat frame, int t, Codebook *codebooks){

	int width = frame.cols;
	int height = frame.rows;

	// Get height and weight from frame
	for(int y=0; y<height; y++) {			// Run the algorithm for each pixel
		for(int x=0; x<width; x++) {

			// Get codebook of current pixel
			Codebook *cb;
			cb = codebooks+y*width+x;

			// Get rgb value of current pixel into rgb[3] array
			Vec3b intensity = frame.at<Vec3b>(y, x);
			double rgb[3];
			rgb[0] = intensity[0];
			rgb[1] = intensity[1];
			rgb[2] = intensity[2];

			// Calculate the brightness
			double I = sqrt(rgb[0]*rgb[0] + rgb[1]*rgb[1] + rgb[2]*rgb[2]);

			// Seek matched codewords
			int matched = -1;
			for( int i=0; i<cb->cwlist.size(); i++ ) {
				if( isMatched(cb->cwlist.at(i), rgb, I) ) {
					matched = i;
					break;
				}
			}

			if (matched == -1) {		// There is no match or codebookSize == 0, add new codeword
				Codeword *cw = new Codeword;

				cw->rgb[0] = rgb[0];
				cw->rgb[1] = rgb[1];
				cw->rgb[2] = rgb[2];
				cw->minI = I;
				cw->maxI = I;
				cw->f 	 = 1;
				cw->l 	 = t-1;
				cw->p	 = t;
				cw->q	 = t;

				cb->cwlist.push_back(cw);
				cwCount++;
			}
			else {						// Update matched codeword
				Codeword *cw = cb->cwlist.at(matched);
				int f = cw->f;
				int fn = f++;

				cw->rgb[0] = (f*cw->rgb[0]+rgb[0])/fn;
				cw->rgb[1] = (f*cw->rgb[1]+rgb[1])/fn;
				cw->rgb[2] = (f*cw->rgb[2]+rgb[2])/fn;
				cw->minI = min(cw->minI, I);
				cw->maxI = min(cw->maxI, I);
				cw->f = fn;
				cw->l = max(cw->l, t-cw->q);
				//cw->p = cw->p;	// Redundant
				cw->q = t;
			}

		}
	}

}

bool Codebook::isMatched( Codeword *cw, double rgb[3], double I  ) {
	//bool colorMatch = true;
	//bool brigthMatch = true;

	double xt_sqr = rgb[0]*rgb[0] + rgb[1]*rgb[1] + rgb[2]*rgb[2];
	double vi_sqr = cw->rgb[0]*cw->rgb[0] + cw->rgb[1]*cw->rgb[1] + cw->rgb[2]*cw->rgb[2];
	double xtvi   = rgb[0]*cw->rgb[0] + rgb[1]*cw->rgb[1] + rgb[2]*cw->rgb[2];
	xtvi = xtvi * xtvi;

	double sig = sqrt(xt_sqr - xtvi/vi_sqr);

	double I_low = alpha*cw->maxI;
	double I_hi = min( beta*cw->maxI, cw->minI/alpha );

	return sig <= eps && I_low <= I && I <= I_hi;
}

