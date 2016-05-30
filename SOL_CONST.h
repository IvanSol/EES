#ifndef SOL_CONST
#define SOL_CONST

//#define WIN

#ifndef WIN 
#define LINUX
#endif

#ifdef WIN
	#define SOL_ZERO 0
	#define GRAPHICS_PROHIBITED false
#endif
#ifdef LINUX
	#define SOL_ZERO COL_BLACK
	#define GRAPHICS_PROHIBITED true
#endif

//#define TEST_IN
//#define TEST_OUT
#define TEST_OUT
#define TEST_SHOW
#ifdef TEST_OUT
	std::ofstream TEST("test.txt");
#endif
#ifdef TEST_IN
	std::ifstream TEST("test.txt");
#endif
int testcount = 0;

#define PI        3.14159265358979323846

#define S_WIDTH 40 //Ўирина сектора в отсчЄтах. ќтсчЄт - 1/512 окружности.
#define POLAR_W 300
#define POLAR_H 60
#define POLAR_CROSS (POLAR_W * (S_WIDTH/2) / 512 + 1)
#define dfi ((double) POLAR_W /(2*PI))

#define CASIA
//#define ICE_DB

#ifdef CASIA
#define BASE_NAME "CASIA"
#define FOLDER_IMG "../CASIA/"
#define DAT_FILE "../data_casia.txt"
#endif

#ifdef ICE_DB
#define BASE_NAME "ICE_DB"
#define FOLDER_IMG "../ICE_DB/"
#define DAT_FILE "../Ice_SD_normdata.txt"
#endif


#include<fstream>
std::ofstream debug("debug_output.txt");
#define eps 1e-3

#define TIMING_NO 0		 //No timing data.
#define TIMING_MINIMAL 1 //Processing time for every image
#define TIMING_FULL 2    //Processing time for every operation
#define TIMING_MAXIMAL 3 //Processing time for every suboperation

#define TIMING_TYPE TIMING_MINIMAL

#endif



