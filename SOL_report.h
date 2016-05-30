#ifndef SOL_REPORT
#define SOL_REPORT

#include <iostream>
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "SOL_classes.h"
#include "SOL_geom.h"
#include "SOL_CONST.h"
#include "SOL_functions.h"

#include <vector>

using namespace std;
using namespace cv;


//TODO: Сделать фактор-тип "Направление".
enum Direction {dir_right, dir_down, dir_left, dir_up, dir_center};
/*#define DIRECTION_RIGHT 0
#define DIRECTION_DOWN 1
#define DIRECTION_LEFT 2
#define DIRECTION_UP 3*/

template <typename T>
int DrawToXY(const Mat & src, Mat & dst, int x, int y, bool draw_frame = true) //Работает только для изображений одинаковой размерности (С пикселем типа T)
{
	if (draw_frame)
	{
		x += 1;
		y += 1;
		line(dst, cv::Point(x-1, y-1), cv::Point(x + src.cols, y-1), COL_WHITE);
		line(dst, cv::Point(x-1, y), cv::Point(x - 1, y + src.rows), COL_WHITE);
		line(dst, cv::Point(x + src.cols, y), cv::Point(x + src.cols, y + src.rows), COL_WHITE);
		line(dst, cv::Point(x-1, y + src.rows), cv::Point(x + src.cols, y + src.rows), COL_WHITE);
	}

	if ((x + src.cols > dst.cols) || (y + src.rows > dst.rows))
		return -1;
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++)
			dst.at<T>(y + i, x + j) = src.at<T>(i, j);
	return 0;
}

const int interval_horisontal = 30;
const int interval_vertical = 50;
const int interval_vertical_section = 50;
const int interval_text = 5; 
const int interval_margin = 5;
const int interval_frame = 5;

class rep_element
{
public:
	rep_element()
	{
		position = dir_center;
		w = 0;
		h = 0;
	};
	Direction position;
	int w;
	int h;
	virtual void draw(Mat & dst, int x, int y)
	{
		return;
	};
};

class rep_Img : public rep_element
{
public:
	rep_Img(const Mat & _img, const string & _name, const Scalar & _color = COL_WHITE, const int _thickness = 1, Direction _position = dir_center)
	{
		_img.copyTo(img);
		name = _name;
		color = _color;
		thickness = _thickness;
		position = _position;
		int y_shift;
		Size sz = getTextSize(name, FONT_HERSHEY_SIMPLEX, 1.0, thickness, &y_shift);
		h = img.rows + sz.height + interval_text;
		w = max(img.cols, sz.width);
	};
	Mat img;
	string name;
	Scalar color;
	int thickness;
	virtual void draw(Mat & dst, int x, int y)
	{
		int y_shift;
		Size sz = getTextSize(name, FONT_HERSHEY_SIMPLEX, 1.0, thickness, &y_shift);
		putText(dst, name, cv::Point(x, y + sz.height), FONT_HERSHEY_SIMPLEX, 1.0, COL_WHITE, thickness);
		DrawToXY<uchar>(img, dst, x, y + sz.height + interval_text, true);
	};
};

class rep_Text : public rep_element
{
public:
	rep_Text(const string & _text, const Scalar & _color = COL_WHITE, const int _thickness = 1, Direction _position = dir_center)
	{
		text = _text;
		color = _color;
		thickness = _thickness;
		position = _position;
		int y_shift;
		Size sz = getTextSize(text, FONT_HERSHEY_SIMPLEX, 1.0, thickness, &y_shift);
		h = sz.height;
		w = sz.width;
	}
	string text;
	Scalar color;
	int thickness;
	
	virtual void draw(Mat & dst, int x, int y)
	{
		int y_shift;
		Size sz = getTextSize(text, FONT_HERSHEY_SIMPLEX, 1.0, thickness, &y_shift);
		putText(dst, text, cv::Point(x, y + sz.height), FONT_HERSHEY_SIMPLEX, 1.0, COL_WHITE, thickness);
	}
};

//TODO: Создать класс "колонка".
class rep_column
{
public:
	vector<rep_element*> data;
	Size GetSize()
	{
		Size res;
		res.height = 0;
		res.width = 0;
		for (int i = 0; i < data.size(); i++)
		{
			res.height += data[i]->h;
			if (data[i]->w > res.width)
				res.width = data[i]->w;
		}
		res.height += interval_vertical * (data.size() - 1);
		return res;
	}

	int draw(Mat & dst, int x0, int y0, int h)
	{
		Size sz = GetSize();
		int dy = (h - sz.height)/(data.size());
		int y = y0;

		for (int i = 0; i < data.size(); i++)
		{
			data[i] -> draw(dst, x0, y+dy/2);
			y += data[i] -> h + dy + interval_vertical;
		}
		return sz.width;
	}

	void AddImg(const Mat & _img, const string & _name = "", const int _thickness = 1, const Scalar & _color = COL_WHITE, Direction _position = dir_center)
	{
		data.push_back(new rep_Img(_img, _name, _color, _thickness, _position));
	}
	void AddText(const string & _text, const int _thickness = 1, const Scalar & _color = COL_WHITE, Direction _position = dir_center)
	{
		data.push_back(new rep_Text(_text, _color, _thickness, _position));
	}

	~rep_column()
	{
		for (int i = 0; i < data.size(); i++)
			free(data[i]);
	}
};

class rep_section
{
public:
	virtual int draw(Mat & dst, int x0, int y0, int w) = 0; //Возвращает высоту нарисованной секции.
	virtual Size GetSize() = 0;
	virtual void AddImg(const Mat & _img, const string & _name = "", const int _thickness = 1, const Scalar & _color = COL_WHITE) = 0;
	virtual void AddText(const string & _text, const int _thickness = 1, const Scalar & _color = COL_WHITE) = 0;
	virtual void NewColumn() = 0;
};

class rep_header_section:public rep_section
{
private:
	Scalar col;
	String left, right, mid;
public:
	virtual int draw(Mat & dst, int x0, int y0, int w) //Возвращает высоту нарисованной секции.
	{
		int y_shift;
		Size sz1 = getTextSize(left, FONT_HERSHEY_SIMPLEX, 1.0, 1, &y_shift);
		Size sz2 = getTextSize(mid, FONT_HERSHEY_SIMPLEX, 1.0, 2, &y_shift);
		Size sz3 = getTextSize(right, FONT_HERSHEY_SIMPLEX, 1.0, 1, &y_shift);
		int h = max(max(sz1.height, sz2.height), sz3.height);
		putText(dst, left, cv::Point(x0, y0 + sz1.height), FONT_HERSHEY_SIMPLEX, 1.0, col, 1);
		putText(dst, mid, cv::Point(x0 + w/2 - sz2.width/2, y0 + sz2.height), FONT_HERSHEY_SIMPLEX, 1.0, col, 2);
		putText(dst, right, cv::Point(x0 + w - sz3.width, y0 + sz3.height), FONT_HERSHEY_SIMPLEX, 1.0, col, 1);
		return h;
	}
	virtual Size GetSize()
	{
		Size res;
		int y_shift;
		Size sz1 = getTextSize(left, FONT_HERSHEY_SIMPLEX, 1.0, 1, &y_shift);
		Size sz2 = getTextSize(mid, FONT_HERSHEY_SIMPLEX, 1.0, 2, &y_shift);
		Size sz3 = getTextSize(right, FONT_HERSHEY_SIMPLEX, 1.0, 1, &y_shift);
		res.height = max(max(sz1.height, sz2.height), sz3.height);
		res.width = sz1.width + sz2.width + sz3.width + 2*interval_horisontal;
		return res;
	}
	rep_header_section(String _mid, String _left = "", String _right = "", Scalar _col = COL_WHITE)
	{
		mid = _mid;
		left = _left;
		right = _right;
		col = _col;
	}
	virtual void AddImg(const Mat & _img, const string & _name = "", const int _thickness = 1, const Scalar & _color = COL_WHITE)
	{
		return;
	}
	virtual void AddText(const string & _text, const int _thickness = 1, const Scalar & _color = COL_WHITE)
	{
		return;
	}
	virtual void NewColumn()
	{
		return;
	}
	~rep_header_section();
};

class rep_graph_section:public rep_section
{
public:
	vector<rep_column*> data;
	virtual int draw(Mat & dst, int x0, int y0, int w) //Возвращает высоту нарисованной секции.
	{
		Size sz = GetSize();
		int h = sz.height;
		int dx = 0;
		if (data.size() > 1)
			dx = (w - sz.width)/(data.size() - 1);
		int x = x0;
		for (int i = 0; i < data.size(); i++)
			x += data[i]->draw(dst, x, y0, h) + interval_horisontal + dx;
		return h;
	}

	virtual Size GetSize()
	{
		Size res;
		res.height = 0;
		res.width = 0;
		for (int i = 0; i < data.size(); i++)
		{
			Size sz = data[i]->GetSize();
			res.width += sz.width;
			if (res.height < sz.height)
				res.height = sz.height;
		}
		res.width += interval_horisontal * (data.size() - 1);
		return res;
	}

	virtual void AddImg(const Mat & _img, const string & _name = "", const int _thickness = 1, const Scalar & _color = COL_WHITE)
	{
		data[data.size()-1]->AddImg(_img, _name, _thickness, _color);
	}
	virtual void AddText(const string & _text, const int _thickness = 1, const Scalar & _color = COL_WHITE)
	{
		data[data.size()-1]->AddText(_text, _thickness, _color);
	}
	/*void AddEmptyCol()
	{
		data.push_back(new rep_column);
	}*/
	virtual void NewColumn()
	{
		data.push_back(new rep_column);
	}
	~rep_graph_section()
	{
		for (int i = 0; i < data.size(); i++)
			free(data[i]);
	}
};

class SOL_report
{
private:
	vector<rep_section*> data;
public:
	String top_mid, bot_mid, top_left, bot_left, top_right, bot_right;

	SOL_report()
	{
		NewSection();
	}
	void AddImg(const Mat & _img, const string & _name = "", const int _thickness = 1, const Scalar & _color = COL_WHITE)
	{
		data[data.size() - 1]->AddImg(_img, _name, _thickness, _color);
	};
	void AddText(const string & _text, const int _thickness = 1, const Scalar & _color = COL_WHITE)
	{
		data[data.size() - 1]->AddText(_text, _thickness, _color);
	};
	/*void AddEmptyCol()
	{
		data[data.size() - 1]->AddEmptyCol();
	};*/
	void NewColumn()
	{
		data[data.size() - 1]->NewColumn();
	};
	void NewSection()
	{
		data.push_back(new rep_graph_section);
		NewColumn();
	};
	void AddHeader(String mid, String left="", String right="", Scalar col = COL_WHITE)
	{
		if (data[data.size()-1]->GetSize().height == 0)
			data.pop_back();
		data.push_back(new rep_header_section(mid, left, right, col));
		//NewSection();
	}
	Mat generate(int w_all = 1250, int h_all = 750)
	{
		int w_max = 0;
		int h = 2 * interval_margin;
		for (int i = 0; i < data.size(); i++)
		{
			Size sz = data[i]->GetSize();
			if (sz.width > w_max)
				w_max = sz.width;
			h += sz.height + interval_vertical_section;
		}
		int w = 2 * interval_frame + w_max;
		h -= interval_vertical_section;
		Mat res(h_all, w_all, CV_8UC1);
		res = COL_BLACK;
		int y = (h_all - h)/2 + interval_margin;
		int x = (w_all - w)/2 + interval_frame;
		for (int i = 0; i < data.size(); i++)
			y += data[i]->draw(res, x, y, w) + interval_vertical_section;
		return res;
	}
	~SOL_report()
	{
		for (int i = 0; i < data.size(); i++)
			free(data[i]);
	}
};

template <typename T>
Mat Concatinate(const Mat &src1, const Mat &src2, Direction pos)
{
	Mat res;
//	IMShow(src1);
//	IMShow(src2);
	if (pos == dir_up || pos == dir_down)
	{
		if (src1.cols != src2.cols)
			cout << "Concatination error!";
		res = Mat(src1.rows + src2.rows, src1.cols, CV_8UC1);
		if (pos == dir_up)
		{
			DrawToXY<T>(src2, res, 0, 0, false);
			DrawToXY<T>(src1, res, 0, src2.rows, false);
		}
		else
		{
			DrawToXY<T>(src1, res, 0, 0);
			DrawToXY<T>(src2, res, 0, src1.rows, false);
		}
	}
	if (pos == dir_left || pos == dir_right)
	{
		if (src1.rows != src2.rows)
			cout << "Concatination error!";
		
		res = Mat(src1.rows, src1.cols + src2.cols, CV_8UC1);
		if (pos == dir_left)
		{
			DrawToXY<T>(src2, res, 0, 0, false);
			DrawToXY<T>(src1, res, src1.cols, 0, false);
		}
		else
		{
			DrawToXY<T>(src1, res, 0, 0, false);
			DrawToXY<T>(src2, res, src2.cols, 0, false);
		}
	}
	return res;
}

template <typename T>
Mat plot_vector(const std::vector<T> &v, int h, int w, int shift, const Scalar &color = COL_WHITE, const Scalar &background = COL_BLACK)
{
	//Чёрно-белая картинка:
	Mat res(h, w, CV_8UC1);
	res = background;

	T vmax = v[0];
	for (int i = 1; i < v.size(); i++)
		if (v[i] > vmax)
			vmax = v[i];
	int jprev = floor((double) v[0] * (h-1) / vmax);
	for (int i = 0; i < v.size(); i++)
	{
		int j = floor((double) v[i] * (h-1) / vmax);
		line(res, cv::Point(shift + i-1, jprev), cv::Point(shift + i, j), color);
		jprev = j;
	}
	return res;
}

template <typename T>
Mat plot_vector_log(const std::vector<T> &v, int h, int w, int shift, const Scalar &color = COL_WHITE, const Scalar &background = COL_BLACK)
{
	Mat res(h, w, CV_8UC1);
	res = background;

	double vmax = log((double) v[0]);
	double vmin = log((double) v[0]);
	for (int i = 1; i < v.size(); i++)
	{
		if (log((double) v[i]) > vmax)
			vmax = log((double) v[i]);
		if (log((double) v[i]) < vmin)
			vmin = log((double) v[i]);
	}
	int jprev = h - 1 - floor((log((double) v[0]) - vmin) * (h-1) / (vmax-vmin));
	for (int i = 1; i < v.size(); i++)
	{
		int j = h - 1 - floor((log((double) v[i]) - vmin) * (h-1) / (vmax-vmin));
		line(res, cv::Point(shift + i-1, jprev), cv::Point(shift + i, j), color);
		jprev = j;
	}
	return res;
}


Size getSimpleTextSize(const string & s, int thickness = 1)
{
	return getTextSize(s, FONT_HERSHEY_SIMPLEX, 1.0, thickness, NULL);
}

void SimpleText(Mat & img, const string s, int x, int y, bool rize_up = true, int thickness = 1) // входная точка - координаты левого нижнего угла.
{
	int interval_text = 0;
	if (rize_up)
		interval_text = 10;
	putText(img, s, cv::Point(x, y - interval_text), FONT_HERSHEY_SIMPLEX, 1.0, COL_WHITE, thickness);
}

/*Mat SectorReport()
{

}*/

Mat MakeReport(const string & filename, const Mat & iris_decart, const Mat & iris, const Mat & plot,         const Mat & mahalanobis, const Mat & mask_pure,
			   const Mat & mask_morph,  const Mat & mask_lacunas, const Mat & mask_applied, const Mat & Result, const double & err)
{
	int margin = 5;
	int interval_text = 10;
	Size sz = getSimpleTextSize(filename + ":", 2);
	int y0 = 50 + margin + sz.height;
	int w = 1400;
	int h = 600;
	int w_real = iris_decart.cols + 2*iris.cols + Result.cols + 3*interval_horisontal;
	int x0 = (w - w_real) / 2;
	//int w = iris_decart.cols + 2*iris.cols + Result.cols + 3*interval_horisontal + 2*x0;
	//int h = 2*y0 + margin + iris.rows * 4 + 3*interval_vertical;
	Mat res(h, w, CV_8UC1);
	res = COL_BLACK;

	SimpleText(res, filename + ":", w/2 - sz.width/2, margin + sz.height, false, 2);

	int x = x0;
	int y = h/2 - iris_decart.rows / 2;

	SimpleText(res, "Source:", x, y);
	DrawToXY<uchar>(iris_decart, res, x, y);

	x += iris_decart.cols + interval_horisontal;	
	y = y0;

	SimpleText(res, "Polar transform:", x, y);
	DrawToXY<uchar>(iris, res, x, y);
	y += iris.rows + interval_vertical;

	SimpleText(res, "Excess plot:", x, y);
	DrawToXY<uchar>(plot, res, x, y);
	y += 2*iris.rows + 2*interval_vertical;

	SimpleText(res, "Mahalanobis:", x, y);
	DrawToXY<uchar>(mahalanobis, res, x, y);

	x += iris.cols + interval_horisontal;
	y = y0;

	SimpleText(res, "Pure mask:", x, y);
	DrawToXY<uchar>(mask_pure, res, x, y);
	y += iris.rows + interval_vertical;
	SimpleText(res, "Morphology:", x, y);
	DrawToXY<uchar>(mask_morph, res, x, y);
	y += iris.rows + interval_vertical;
	SimpleText(res, "Lacunas detection:", x, y);
	DrawToXY<uchar>(mask_lacunas, res, x, y);
	y += iris.rows + interval_vertical;
	SimpleText(res, "Applied mask:", x, y);
	DrawToXY<uchar>(mask_applied, res, x, y);

	x += mask_pure.cols + interval_horisontal;
	y = h/2 - Result.rows / 2;
	SimpleText(res, "Result:", x, y);
	DrawToXY<uchar>(Result, res, x, y);

	std::stringstream str_str;
	str_str << err;
	sz = getSimpleTextSize("Error = " + str_str.str(), 2);
	SimpleText(res, "Error = " + str_str.str(), w/2 - sz.width/2, h - margin, true, 2);

	sz = getSimpleTextSize((string) "Image base: " + BASE_NAME);
	SimpleText(res, (string) "Image base: " + BASE_NAME, w - sz.width - 5, h - margin);

	return res;
}

#endif