#ifndef SOL_PARAMETERS
#define SOL_PARAMETERS

#include <iostream>
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "SOL_CONST.h"
using namespace std;
using namespace cv;

//Все функции вычисления параметров должны иметь сигнатуру: 
//int (
//		const Mat &,	//source image
//		const Pnt &,	//point, where to calculate parameter
//		double*,		//where to put result
//		void* = NULL	//additional data
//	  ). Возвращает количество вычисленных параметров.
//
typedef int (*ParamCalcFunction) (const Mat &, const Pnt &, double*, void*);

#define PARAM_A_2 5 
#define PARAM_A (PARAM_A_2 * 2 + 1)
#define PARAM_B_2 5
#define PARAM_B (PARAM_B_2 * 2 + 1)
#define FRAME PARAM_A/2+1
//const int FRAME = max(max(PARAM_A/2+1, PARAM_B/2+1), 5);

int B(const Mat & img, const Pnt & p0, double* dst, void* data = NULL)
{
	(*dst) = Pnt(p0).GetBright(img);
	return 1;
}

int AvgB(const Mat & img, const Pnt & p0, double* dst, void* data = NULL)
{
	int x0 = p0.x - PARAM_A_2 - 1;
	int y0 = p0.y - PARAM_A_2 - 1;
	double res = 0;
	for (int x = x0; x < x0 + PARAM_A; x++)
		for (int y = y0; y < y0 + PARAM_A; y++)
		{
			//Pnt p0(x0, y0);
			//res += p0.GetBright(img);
			//test(img.at<uchar>(y, x), "Mrk at x = " + IntToStr(p0.x) + ", y = " + IntToStr(p0.y) + ": value at point x = " + IntToStr(x) + ", y = " + IntToStr(y));
			res += img.at<uchar>(y, x);
		}
	(*dst) = res / (PARAM_A*PARAM_A);
	return 1;
}

int SigmaB(const Mat & img, const Pnt & p0, double* dst, void* data = NULL)
{
	int x0 = p0.x - PARAM_B_2 - 1;
	int y0 = p0.y - PARAM_B_2 - 1;
	double res = 0;
	for (int x = x0; x < x0 + PARAM_B; x++)
		for (int y = y0; y < y0 + PARAM_B; y++)
		{
			//Pnt p0(x0, y0);
			//res += sqr(p0.GetBright(img));
			res += sqr(img.at<uchar>(y, x));
			//debug << (int) img.at<uchar>(y0, x0) << " ";
		}
	//return sqrt(res / (b*b) - sqr(AvgB(img, p0, PARAM_B_2)));
	//в data передаётся среднее значение яркости в данной окрестности
	(*dst) = sqrt(res / (PARAM_B*PARAM_B) - *((double*) data) * *((double*) data));
	return 1;
}

int DCT5(const Mat & img, const Pnt & p0, double* dst, void* data = NULL)
{
	Rect wind(p0.x - 3, p0.y - 3, 8, 8);
	SimpleFMat res(8, 8);
	img(wind).convertTo(res.data, CV_64F);
	dct(res.data, res.data); //Размер изображения должен быть степенью 2 ???!!!
	//IMShow(res);
	dst[0] = res[0][1];
	dst[1] = res[1][0];
	dst[2] = res[0][2];
	dst[3] = res[1][1];
	dst[4] = res[2][0];
	return 5;
}

int Markov(const Mat & img, const Pnt & p0, double* dst, void* data = NULL)
{
	int x0 = p0.x - PARAM_A_2 - 1;
	int y0 = p0.y - PARAM_A_2 - 1;
	Mat res(PARAM_A, PARAM_A, CV_8UC1);
	//Применение бинаризации по порогу Оцу:
	threshold(img(Rect(x0, y0, PARAM_A, PARAM_A)), res, 255, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	//IMShow(img(Rect(x0, y0, PARAM_A, PARAM_A)), "SRC", !SHOW_WAIT);
	//IMShow(res, "DST");
	dst[0] = (dst[1] = (dst[2] = (dst[3] = 0)));
	//пока не учитываю переходы по вертикали (Нужно ли?)
	for (int i = 0; i < PARAM_A - 1; i++)
		for (int j = 0; j < PARAM_A; j++)
			dst[(int) (res.at<uchar>(i, j) == 0 ) * 2 + (int) (res.at<uchar>(i + 1, j) == 0)]++;
	return 4;
}

#endif