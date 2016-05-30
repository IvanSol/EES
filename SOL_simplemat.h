#ifndef SOL_SIMPLE_MAT
#define SOL_SIMPLE_MAT

#include <iostream>
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "SOL_CONST.h"

using namespace std;
using namespace cv;

//Допустимые границы определителя для подсчёта обратной матрицы:
#define DET_L 1e+30
#define DET_R 1e+50

class SimpleFMat
{
public:
	Mat data;
	SimpleFMat(Mat newdata)
	{
		data = newdata;
	}
	SimpleFMat(int rows, int cols)
	{
		data = /**(new */Mat(rows, cols, CV_64FC1)/*)*/;
		///*
		for (int i = 0; i < data.rows; i++)
		{
			for (int j = 0; j < data.cols; j++)
				data.at<double>(i, j) = 0;
		}
		//*/

	}
	~SimpleFMat(void)
	{
		data.~Mat();
		//free(&data);
	}
	double *operator [] (int i)
	{
		return &(*(data.begin<double>() + i * data.step1())); //ТУТ КАКАЯ-ТО ЛАЖА!!!
	}
	friend std::ostream &operator << (std::ostream & os, const SimpleFMat &a)
	{
		for (int i = 0; i < a.data.rows; i++)
		{
			for (int j = 0; j < a.data.cols; j++)
			{
				double t = a.data.at<double>(i, j);
				os << t << "	";
				/*if (&(a.data.at<double>(i, j)) != &(a[i][j]))
					cout << "Index mismatch in SimpleMat. Wrong [] operator!" << endl;*/
			}
			os << endl;
		}
		os << endl;
		return os;
	}
	SimpleFMat operator + (const SimpleFMat & b)
	{
		return SimpleFMat(data + b.data);
	}
	SimpleFMat operator += (const SimpleFMat & b)
	{
		data += b.data;
		return *this;
	}
	SimpleFMat operator - (const SimpleFMat & b)
	{
		return SimpleFMat(data - b.data);
	}
	SimpleFMat operator * (const SimpleFMat & b)
	{
		return SimpleFMat(data * b.data);
	}
	SimpleFMat operator = (const double val)
	{
		for (int i = 0; i < data.rows; i++)
			for (int j = 0; j < data.cols; j++)
				data.at<double>(i, j) = val;
		return *this;
	}
	SimpleFMat operator /= (const double val)
	{
		for (int i = 0; i < data.rows; i++)
			for (int j = 0; j < data.cols; j++)
				data.at<double>(i, j) /= val;
		return *this;
	}
	SimpleFMat operator *= (const double val)
	{
		for (int i = 0; i < data.rows; i++)
			for (int j = 0; j < data.cols; j++)
				data.at<double>(i, j) *= val;
		return *this;
	}
	SimpleFMat t()
	{
		return Mat(data.t());
	}
	SimpleFMat inv()
	{
		double t = abs(determinant(data));
		double mult = 1;
		if (abs(t) < eps)
			mult = 1e-4;			
		while ((abs(t) < eps) && (mult < 1e+4))
		{
			mult *= 2;
			t = abs(determinant(data*mult));
		}
		if (abs(t) < eps)
		{
			cerr << "Error! Bad matrix. Can't inverse." << endl;
			debug << "Error! Bad matrix. Can't inverse." << endl;
			mult = 1;
		}
		return Mat((data*mult).inv() * mult);
	}
};

#endif