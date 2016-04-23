#include "codebook.h"

Codebook::~Codebook(){
    /*Codeword* cw;
    cw = this->cwlist.at(0);
    int r=cw->row;
    int c=cw->col;
    if(r > 280)
        cout << r << "x" << c << endl;*/
	for(unsigned int i=0; i<cwlist.size(); i++)
		delete cwlist.at(i);
    /*if(r > 280)
        cout << r << "x" << c << endl;*/
}

bool Codebook::initCodebooks(RunMode m) {
    bool ret = true;
    mode = m;
    codebooks = new Codebook[ width*height ];
    if(mode == PLAY) {
        outputFrameBytes = new uint8_t[ width*height ];
        ret = load();
    }
    return ret;
}

void Codebook::clearCodebooks() {
    delete [] codebooks;
    if(mode == PLAY) {
		delete [] outputFrameBytes;
    }
}

void Codebook::processFrame(uint8_t* frameBytes, uint16_t t){

    int rawPixel = 0;
	// Get height and weight from frame
	for(int r=0; r<height; r++) {			// Run the algorithm for each pixel
		for(int c=0; c<width; c++) {

			// Get codebook of current pixel
			Codebook *cb;
			cb = codebooks+rawPixel;

			// Get rgb value of current pixel into rgb[3] array
			float rgb[3];
			int rawChannel = 3*rawPixel;
			rgb[0] = frameBytes[rawChannel+2];
			rgb[1] = frameBytes[rawChannel+1];
			rgb[2] = frameBytes[rawChannel+0];


            // Calculate the brightness
            //float I = sqrt(rgb[0]*rgb[0] + rgb[1]*rgb[1] + rgb[2]*rgb[2]);
            float I = (rgb[0] + rgb[1] + rgb[2]);

			// Seek matched codewords
			int matched = -1;
			for( uint16_t i=0; i<cb->cwlist.size(); i++ ) {
				if( isMatched(cb->cwlist.at(i), rgb, I) ) {
					matched = i;
					break;
				}
			}

			if (mode == PLAY){
                // Background substraction
                if (matched == -1) {
                    outputFrameBytes[rawPixel] = 0;
                }
                else {
                    outputFrameBytes[rawPixel] = 255;
                }

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

                    cw->col = c;
                    cw->row = r;
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
                    uint16_t temp = t-cw->q;
                    cw->l = max(cw->l, temp);
                    //cw->p = cw->p;	// Redundant
                    cw->q = t;
                }
            }

            rawPixel++; // rawPixel = r*width+c + 1;
		}
	}

}

void Codebook::applyMNRL(){
	for(int y=0; y<height; y++) {
		for(int x=0; x<width; x++) {
			// Get codebook of current pixel
			Codebook *cb;
			cb = codebooks+y*width+x;
            for(uint32_t i=0; i<cb->cwlist.size(); i++) {
                Codeword *cw = cb->cwlist.at(i);
                uint16_t temp = frameCount - cw->q + cw->p - 1;
                cw->l = max(cw->l, temp);
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
	float p_sqr  = xtvi * xtvi / vi_sqr;
	float sig    = sqrt(xt_sqr - p_sqr);

	float I_low = alpha*cw->maxI;
	float I_hi = min( beta*cw->maxI, cw->minI/alpha );

	return sig <= eps && I_low <= I && I <= I_hi;
}

bool Codebook::fileExists (string filename) {
  struct stat buffer;
  return (stat (filename.c_str(), &buffer) == 0);
}

bool Codebook::save() {

    FILE *ptr_fp;

    if( (ptr_fp = fopen(fileName.c_str(), "wb")) == NULL ) {
        printf("Error: Unable to open file!\n");
        return false;
    }

	Codebook    *cb;
	Codeword    *cw;
	CBFHeader   *header = new CBFHeader;

	header->width   = width;
	header->height  = height;
	header->alpha   = alpha;
	header->beta    = beta;
	header->eps     = eps;
	header->cwCount = cwCount;

    if( fwrite(header, sizeof(CBFHeader), 1, ptr_fp) != 1) {
        printf("Error: Write error!\n");
        return false;
    }

	// Get codebook of current pixel
	// Dump all codewords in it
	for(int y=0; y<height; y++) {
		for(int x=0; x<width; x++) {
			cb = codebooks+y*width+x;
			for( uint32_t i=0; i<cb->cwlist.size(); i++ ) {

                cw = cb->cwlist.at(i);
                if( fwrite(cw, sizeof(Codeword), 1, ptr_fp) != 1) {
                    printf("Error: Write error!\n");
                    return false;
                }
			}
		}
	}
    fclose(ptr_fp);

    delete header;
    cout << "Saved into file " << fileName << endl;

    return true;
}

bool Codebook::load() {

    cout << "Loading from file " << fileName << endl;

    FILE *ptr_fp;

    if( (ptr_fp = fopen(fileName.c_str(), "rb")) == NULL ) {
        printf("Error: Unable to open file!\n");
        return false;
    }

    fseek(ptr_fp,0,SEEK_END);
    uint32_t fsize = ftell(ptr_fp);
    fseek(ptr_fp,0,SEEK_SET);

    CBFHeader * header = new CBFHeader;
    fread ( header, sizeof(CBFHeader), 1, ptr_fp );
    cwCount = header->cwCount;
    alpha   = header->alpha;
    beta    = header->beta;
    height  = header->height;
    width   = header->width;

    if (fsize != sizeof(CBFHeader)+sizeof(Codeword)*cwCount) {
        printf("Error: File is corrupted!\n");
        return false;
    }
    else {

        Codebook *cb;
        while(!feof(ptr_fp)){
            Codeword * cw = new Codeword;
            if(!fread ( cw, sizeof(Codeword), 1, ptr_fp ))
                break;
            cb = codebooks+cw->row*width+cw->col;
            cb->cwlist.push_back(cw);
        }

    }
    delete header;
    fclose(ptr_fp);

    return true;
}

bool Codebook::load(string fn) {
    fileName = fn;
    return load();
}

