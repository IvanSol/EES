#ifndef SOL_MORPHOLOGY
#define SOL_MORPHOLOGY
#include "opencv/cv.h"
//#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;

#include "SOL_classes.h"
#include "SOL_CONST.h"

void opening(Mat &img, int kersize)
{
	Mat ker(kersize, kersize, CV_8UC1);
	ker = 1;
	erode(img, img, ker, cvPoint(kersize/2, kersize/2));
	dilate(img, img, ker, cvPoint(kersize/2, kersize/2));
}

void closing(Mat &img, int kersize)
{
	Mat ker(kersize, kersize, CV_8UC1);
	ker = 1;
	dilate(img, img, ker, cvPoint(kersize/2, kersize/2));
	erode(img, img, ker, cvPoint(kersize/2, kersize/2));
}

const int dx[4] = {1, 0, -1, 0};
const int dy[4] = {0, -1, 0, 1};

void DFS(Mat & colors, const int i, const int j, const int color)
{
	colors.at<uchar>(i, j) = color;
	//IMShow(colors);
	for (int t = 0; t < 4; t++)
		if ((j + dx[t] < colors.cols) && (j + dx[t] >= 0) && (i + dy[t] < colors.rows) && (i + dy[t] >= 0))
			if (colors.at<uchar>(i + dy[t], j + dx[t]) == 0)
				DFS(colors, i + dy[t], j + dx[t], color);
}

int component_lacunas_detection(const Mat & iris, Mat & mask, int frame)
{
	mask(Rect(0, mask.rows - frame, mask.cols, frame)) = 255;
	Mat colors(mask.rows, mask.cols, CV_8UC1);
	colors = SOL_ZERO;
	//colors += 0;

	//IMShow(mask);

	for (int i = 0; i < mask.rows; i++)
		for (int j = 0; j < mask.cols; j++)
			if (mask.at<uchar>(i, j) == 0)
				colors.at<uchar>(i, j) = 1;

	int maxcol = 1;
	for (int i = 0; i < mask.rows; i++)
		for (int j = 0; j < mask.cols; j++)
			if (colors.at<uchar>(i, j) == 0)
				DFS(colors, i, j, ++maxcol);

	vector<int> partial_sums;
	vector<int> color_nums;
	partial_sums.resize(maxcol, 0);
	color_nums.resize(maxcol, 0);

	//НУЖНО ВЫДЕЛИТЬ ИЗ КОМПОНЕНТ ВЕРХНЕЕ И НИЖНЕЕ ЗАТЕНЕНИЯ

	for (int i = 0; i < mask.rows; i++)
		for (int j = 0; j < mask.cols; j++)
		{
			partial_sums[colors.at<uchar>(i, j) - 1] += iris.at<uchar>(i, j);
			color_nums[colors.at<uchar>(i, j) - 1] ++;
		}

	vector<bool> color_ok;
	color_ok.resize(maxcol, true);
	int avg_bright_eps = -5;
	double avg = (double) partial_sums[0] / color_nums[0];
	for (int i = 1; i < maxcol; i++)
	{
		if (avg - (double) partial_sums[i] / color_nums[i] > avg_bright_eps)
			color_ok[i] = false;
	}
	
	//Считаем, что затенения, примыкающие к внешнему краю радужки по умолчанию хорошие.
	color_ok[colors.at<uchar>(colors.rows-1, 0) - 1] = true;

	for (int i = 0; i < mask.rows; i++)
		for (int j = 0; j < mask.cols; j++)
			if (!color_ok[colors.at<uchar>(i, j) - 1])
				mask.at<uchar>(i, j) = 0;
	mask(Rect(0, mask.rows - frame, mask.cols, frame)) = SOL_ZERO;
	return 0;
}

#endif
