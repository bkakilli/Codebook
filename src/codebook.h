#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>

using namespace std;

// 32 byte sized codeword
typedef struct{
    uint16_t w;
    uint16_t h;
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
	static uint16_t frameCount;
	static uint8_t* outputFrameBytes;

	static void initCodebooks(RunMode mode);
	static void endCodebooks();
	static void processFrame(uint8_t* frameBytes, unsigned short  t);
	static void applyMNRL();
	static bool isMatched( Codeword* cw, float rgb[3], float I  );
	static bool fileExists(string fileName);
	static void save();
	static void load();
	static void load(string fn);

};
