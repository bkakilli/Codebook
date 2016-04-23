#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>

using namespace std;

// 32 byte sized codeword
typedef struct{
    uint16_t col;
    uint16_t row;
    uint16_t f;
    uint16_t l;
    uint16_t p;
    uint16_t q;
    float rgb[3];
    float minI;
    float maxI;
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
	vector<Codeword *> cwlist;
    enum RunMode {PLAY=0, TRAIN};
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
	static uint16_t frameCount;
	static uint8_t* outputFrameBytes;

	static bool initCodebooks(RunMode mode);
	static void clearCodebooks();
	static void processFrame(uint8_t* frameBytes, uint16_t  t);
	static void applyMNRL();
	static bool isMatched( Codeword* cw, float rgb[3], float I  );
	static bool fileExists(string fileName);
	static bool save();
	static bool load();
	static bool load(string fn);

};
