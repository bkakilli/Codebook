#include "codebook.h"

Codebook::~Codebook(){
	for(int i=0; i<cwlist.size(); i++)
		delete cwlist.at(i);
}

void Codebook::processFrame(Mat frame, int t){

	// Get height and weight from frame
	for(int y=0; y<height; y++) {			// Run the algorithm for each pixel
		for(int x=0; x<width; x++) {

			// Get codebook of current pixel
			Codebook *cb;
			cb = codebooks+y*width+x;

			// Get rgb value of current pixel into rgb[3] array
			float rgb[3];
			rgb[0] = frame.at<Vec3b>(y, x)[2];
			rgb[1] = frame.at<Vec3b>(y, x)[1];
			rgb[2] = frame.at<Vec3b>(y, x)[0];

			// Calculate the brightness
			float I = sqrt(rgb[0]*rgb[0] + rgb[1]*rgb[1] + rgb[2]*rgb[2]);

			// Seek matched codewords
			int matched = -1;
			for( int i=0; i<cb->cwlist.size(); i++ ) {
				if( isMatched(cb->cwlist.at(i), rgb, I) ) {
					matched = i;
					break;
				}
			}

			if (mode == PLAY){
                // Bacjground substracion
			}
            else if (mode == TRAIN) {
                if (matched == -1) {		// There is no match or codebookSize == 0, add new codeword
                    Codeword *cw = new Codeword;

                    cw->rgb[0]  = rgb[0];
                    cw->rgb[1]  = rgb[1];
                    cw->rgb[2]  = rgb[2];
                    cw->minI    = I;
                    cw->maxI    = I;
                    cw->f       = 1;
                    cw->l       = t-1;
                    cw->p       = t;
                    cw->q       = t;

                    cw->w = width;
                    cw->h = height;
                    cb->cwlist.push_back(cw);
                    cwCount++;
                }
                else {						// Update matched codeword
                    Codeword *cw = cb->cwlist.at(matched);
                    int f = cw->f;
                    int fn = f+1;
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

}

bool Codebook::isMatched( Codeword *cw, float rgb[3], float I  ) {
	//bool colorMatch = true;
	//bool brigthMatch = true;

	float xt_sqr = rgb[0]*rgb[0] + rgb[1]*rgb[1] + rgb[2]*rgb[2];
	float vi_sqr = cw->rgb[0]*cw->rgb[0] + cw->rgb[1]*cw->rgb[1] + cw->rgb[2]*cw->rgb[2];
	float xtvi   = rgb[0]*cw->rgb[0] + rgb[1]*cw->rgb[1] + rgb[2]*cw->rgb[2];
	xtvi = xtvi * xtvi;

	float sig = sqrt(xt_sqr - xtvi/vi_sqr);

	float I_low = alpha*cw->maxI;
	float I_hi = min( beta*cw->maxI, cw->minI/alpha );

	return sig <= eps && I_low <= I && I <= I_hi;
}

void Codebook::save(string fileName) {

    cout << "Saving into file " << fileName << endl;

    FILE *ptr_fp;

    if((ptr_fp = fopen("default.cbf", "ab")) == NULL)
    {
        printf("Unable to open file!\n");
        exit(1);
    }else printf("Opened file successfully for writing.\n");

	Codebook *cb;
	Codeword *cw;
	for(int y=0; y<height; y++) {
		for(int x=0; x<width; x++) {

			// Get codebook of current pixel
			cb = codebooks+y*width+x;
			for( int i=0; i<cb->cwlist.size(); i++ ) {
                cw = cb->cwlist.at(i);

                if( fwrite(cw, sizeof(Codeword), 1, ptr_fp) != 1)
                {
                    printf("Write error!\n");
                    exit(1);
                }
			}
		}
	}
    fclose(ptr_fp);

    cout << "Binary saved." << fileName << endl;

}

void Codebook::load(string fileName) {
    // To be implemented
}

