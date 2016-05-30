#ifndef SOL_CLASSES
#define SOL_CLASSES

#include "SOL_CONST.h"

#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv/cv.h"
#include <iostream>
#include <fstream> 
#include <string> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;
using namespace cv;

#define OCCLUSION_POINTS_NUM 20
#define SHOW_HOLD true
#define SHOW_WAIT true

#define COL_BLACK	CV_RGB(0, 0, 0)
#define COL_WHITE	CV_RGB(255, 255, 255)
#define COL_GREY	CV_RGB(128, 128, 128)
#define COL_RED		CV_RGB(255, 0, 0)
#define COL_GREEN	CV_RGB(0, 255, 0)
#define COL_BLUE	CV_RGB(0, 0, 255)
#define COL_GRAY(i) CV_RGB(i, i, i )
#define COL_DEFAULT COL_BLUE

#define OCCLUSION_INPUT_SORTED false

long long sqr(const long long &x)
{
	return x*x;
}

int sqr(const int &x)
{
	return x*x;
}

double sqr(const double &x)
{
	return x*x;
}

long double RGB_to_gray(const Scalar &col)
{
	long double res = 0;
	for (int i = 0; i < 4; i++)
		if (col.val[i] > res)
			res = ceil(min(col.val[i], 255.0));
	return res;
}

//Точка: -->
//Определение:
class Pnt
{
public:
	int x;
	int y;

	Pnt(int _x = 0, int _y = 0);
	cv::Point to_cvPoint();
	void draw(Mat &img, Scalar color = COL_DEFAULT, bool rewrite = true);
	Scalar GetColor(const Mat & img);
	uchar GetBright(const Mat & img);
	long double GetValue(const Mat & img);
	void move(const Pnt & v);

	friend std::istream & operator >>(std::istream & is, Pnt & p);
	friend std::ostream & operator <<(std::ostream & os, const Pnt & p);
	friend bool operator ==(const Pnt & a, const Pnt & b);
	friend bool operator !=(const Pnt & a, const Pnt & b);
	friend bool operator < (const Pnt & a, const Pnt & b); //сортировка по x, затем по y.
};


//Конструктор:
Pnt::Pnt(int _x, int _y)
{
	x = _x;
	y = _y;
}
//Конвертация в cvPoint:
cv::Point Pnt::to_cvPoint()
{
	return cv::Point(x, y);
	/*cv::Point t(x, y);
	t.x = x;
	t.y = y;
	return t;*/
}
//Рисование:
void Pnt::draw(Mat &img, Scalar color, bool rewrite)
{
	if (img.channels() == 3)
	{
		for (int i = 0; i < img.channels(); i++)
			if (rewrite)
				img.at<Vec3b>(to_cvPoint())[i] = color.val[i];
			else
				img.at<Vec3b>(to_cvPoint())[i] += color.val[i];
	}
	else if (img.channels() == 1)
	{
		if (rewrite)
		{
			uchar x = RGB_to_gray(color);
			img.at<uchar>(to_cvPoint()) = x; //?
		}
		else
			img.at<uchar>(to_cvPoint()) += color.val[0];
	}
	else
		cerr << "Error! Can't iterate. Number of channels = " << img.channels() << endl;
}
//Смещение:
void Pnt::move(const Pnt & v)
{
	x -= v.x;
	y -= v.y;
}
Scalar Pnt::GetColor(const Mat & img)
{
	if (img.channels() == 3)
	{
		return Scalar(img.at<Vec3b>(to_cvPoint()));
	}
	else if (img.channels() == 1)
	{
		return img.at<uchar>(to_cvPoint());
	}
	else
	{
		cerr << "Error! Can't iterate. Number of channels = " << img.channels() << endl;
		return COL_BLACK;
	}

}
uchar Pnt::GetBright(const Mat & img)
{
	return img.at<uchar>(to_cvPoint());
	//Scalar tmp = img.at<uchar>(to_cvPoint());
	//return tmp.val[0];
//	return RGB_to_gray(GetColor(img)); //Проверить правильность
}

long double Pnt::GetValue(const Mat & img)
{
	return img.at<long double>(to_cvPoint());
/*	Scalar tmp = img.at<double>(to_cvPoint());
	return tmp.val[0];*/
}

//Чтение:
std::istream & operator >>(std::istream & is, Pnt & p)
{
	int x, y;
	is >> x >> y;
	p.x = x;
	p.y = y;
	return is;
}
//Вывод:
std::ostream & operator <<(std::ostream & os, const Pnt & p)
{
	os << "x = " << p.x << ", y = " << p.y;
	return os;
}
//Сравнение:
bool operator ==(const Pnt & a, const Pnt & b)
{
	return ((a.x == b.x) && (a.y == b.y));
}
bool operator !=(const Pnt & a, const Pnt & b)
{
	return ((a.x != b.x) || (a.y != b.y));
}
bool operator < (const Pnt & a, const Pnt & b)
{
	if (a.x != b.x)
		return a.x < b.x;
	else
		return a.y < b.y;
}
//Точка <--

//Кружок: -->
//Определение:
class Circ
{
public:
	Pnt c;
	int R;

	Circ();
	Circ(Pnt _c, int _R);
	Circ(int _x, int _y, int _R);
	void draw(Mat & img, Scalar color = COL_DEFAULT, int thickness = 1, int lineType = 8, int shift = 0);
	void move(const Pnt & v);
	bool inside(const Pnt & p) const;
	
	friend std::istream & operator >>(std::istream & is, Circ & c);
	friend std::ostream & operator <<(std::ostream & os, const Circ & c);
};
//Конструкторы:
Circ::Circ()
{
	c = Pnt();
	R = 0;
}
Circ::Circ(Pnt _c, int _R)
{
	c = _c;
	R = _R;
}
Circ::Circ(int _x, int _y, int _R)
{
	c = Pnt(_x, _y);
	R = _R;
}
//Рисование:
void Circ::draw(Mat & img, Scalar color, int thickness, int lineType, int shift)
{
	circle(img, c.to_cvPoint(), R, color, thickness, lineType, shift);
}
//Смещение:
void Circ::move(const Pnt & v)
{
	c.move(v);
}
//Лежит ли точка в круге:
bool Circ::inside(const Pnt & p) const
{
	return sqr(p.x - c.x) + sqr(p.y - c.y) <= R*R;
}
//Чтение:
std::istream & operator >>(std::istream & is, Circ & c)
{
	is >> c.c >> c.R;
	while ((c.c.x <= c.R) || (c.c.y <= c.R))
		c.R--;
	return is;
}
//Вывод:
std::ostream & operator <<(std::ostream & os, const Circ & c)
{
	os << c.c << ", R = " << c.R;
	return os;
}
//Кружок <--




//Затенение: -->
//Определение:
class Occlusion
{
private:
	vector<Pnt> points; //Поддерживается отсортированным по x, затем по y.
public:
	void draw(Mat & img, Scalar color = COL_DEFAULT, int thickness = 1, int lineType = 8, int shift = 0);
	bool point_is_above(const Pnt & p);
	void move(const Pnt & v);
	int size();
	friend std::istream & operator >>(std::istream & is, Occlusion & occ);
	friend std::ostream & operator <<(std::ostream & os, const Occlusion & occ);
};

int Occlusion::size()
{
	return points.size();
}

bool Occlusion::point_is_above(const Pnt & p)
{
	if (points.size() == 0)
	{
		debug << "Error! Bad occlusuion." << endl;
		return true;
	}

	//Учтено, что ось y инвертирована. "Выше" означает, что y меньше.
	int i = 0;
	if (p.x <= points[0].x)
	{
		//cerr << "Bad occlusion!" << endl;
		return p.y < points[0].y;
	}

	while (i < (int) points.size() - 1 && p.x > points[i+1].x)
		i++;

	if (i == (int) points.size() - 1)
		return p.y < points[i].y;
	else
	{
		double x1 = points[i].x;
		double y1 = points[i].y;
		double x2 = points[i+1].x;
		double y2 = points[i+1].y;
		double y = (double) (y2 - y1) * (- x1 + p.x) / (x2 - x1) + y1;
		return p.y < y;
	}
}

//Рисование:
void Occlusion::draw(Mat & img, Scalar color, int thickness, int lineType, int shift)
{
	for (int i = 0; i < (int) points.size() - 1; i++)
		line(img, points[i].to_cvPoint(), points[i + 1].to_cvPoint(), color, thickness, lineType, shift);
}
//Смещение:
void Occlusion::move(const Pnt & v)
{
	for (int i = 0; i < (int) points.size(); i++)
		points[i].move(v);
}


//Чтение:
std::istream & operator >>(std::istream & is, Occlusion & occ)
{
	Pnt tmp;
	Pnt p0(0, 0);
	is >> tmp;
	for (int i = 0; i < OCCLUSION_POINTS_NUM-1; i++)
	{
		if (tmp != p0)
			occ.points.push_back(tmp);
		is >> tmp;
	}
	if (!OCCLUSION_INPUT_SORTED)
		sort(occ.points.begin(), occ.points.end());
	return is;
}
//Вывод:
std::ostream & operator <<(std::ostream & os, const Occlusion & occ)
{
	for (int i = 0; i < occ.points.size(); i++)
		os << i << ": " << occ.points[i] << endl;
	os << endl;
	return os;
}
//Затенение: <--


//Экспертная разметка: -->
//Определение:
class Expert
{
private:
	Occlusion top;
	Occlusion bottom;
	bool top_empty;
	bool bottom_empty;
public:
	bool point_is_inside(const Pnt &p);
	void draw(Mat & img, Scalar color = COL_DEFAULT, int thickness = 1, int lineType = 8, int shift = 0);
	void move(const Pnt & v);

	friend std::istream & operator >>(std::istream & is, Expert & exp);
	friend std::ostream & operator <<(std::ostream & os, const Expert & exp);
};
//Чтение:
std::istream & operator >>(std::istream & is, Expert & exp)
{
	is >> exp.top >> exp.bottom;
	exp.top_empty = (exp.top.size() <= 1);
	exp.bottom_empty = (exp.bottom.size() <= 1);
	return is;
}
//Вывод:
std::ostream & operator <<(std::ostream & os, const Expert & exp)
{
	os << "Top:" << endl << exp.top << endl << "Bottom:" << endl << exp.bottom << endl;
	return os;
}
//Проврка точки:
bool Expert::point_is_inside(const Pnt &p)
{
	if (!top_empty && !bottom_empty)
		return (top.point_is_above(p)) || (!bottom.point_is_above(p));
	else if (!top_empty)
		return top.point_is_above(p);
	else if (!bottom_empty)
		return !bottom.point_is_above(p);
	else
		return true;
}
//Рисование:
void Expert::draw(Mat & img, Scalar color, int thickness, int lineType, int shift)
{
	top.draw(img, color, thickness, lineType, shift);
	bottom.draw(img, color, thickness, lineType, shift);
}
//Смещение:
void Expert::move(const Pnt & v)
{
	top.move(v);
	bottom.move(v);
}
//Экспертная разметка: <--

#endif