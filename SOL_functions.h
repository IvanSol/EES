#ifndef SOL_FUNCTIONS
#define SOL_FUNCTIONS
//#ifdef SOL_FUNCTIONS
#include <iostream>
#include "opencv/cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "SOL_classes.h"
#include "SOL_geom.h"
#include "SOL_report.h"
#include "SOL_CONST.h"

#include <vector>

using namespace std;
using namespace cv;

#define DIRECTION_RIGHT 0
#define DIRECTION_DOWN 1
#define DIRECTION_LEFT 2
#define DIRECTION_UP 3

template <typename T>
string ToStr(T x)
{
	std::stringstream str_str;
	str_str << x;
	return str_str.str();
}

template <typename T>
void test(const T &x, const string desc = "")
{
#ifdef TEST_NO
	return;
#endif
	testcount++;
#ifdef TEST_OUT
	TEST << (double) x << " ";
#endif
#ifdef TEST_IN
	double t;
	TEST >> t;
	if (abs(t - x) >= 1e-3)
	{
		cout << "TEST FAILED! At testcount = " << testcount;
		cout << ". At " << desc << ": " << (double) t << " != " << (double) x;
		cout << endl;
//		exit(0);
//		x = t;
	}
	#ifdef TEST_SHOW
		cout << "Test OK. At " << desc << ": " << t << " == " << x << endl;
	#endif
#endif
}

//Возвращает код нажатой клавиши:
int IMShow(const Mat & img, const string win_name = "output", bool wait = true, bool hold = true, std::ostream * os = NULL)
{
	if (GRAPHICS_PROHIBITED)
		return 0;

	// окно для отображения картинки
	namedWindow(win_name, CV_WINDOW_AUTOSIZE);
    // показываем картинку
    imshow(win_name, img);
    // выводим в консоль информацию о картинке
	if (os != NULL)
	{
		(*os) << "OUTPUT \"" << win_name << "\":" << endl;
		(*os) << "width:       " << img.cols << " pixels" << endl;
		(*os) << "height:      " << img.rows << " pixels" << endl;
		(*os) << "channels:    " << img.channels() << endl;
		(*os) << "pixel depth: " << img.depth() << " bits" << endl;
		(*os) << "image size:  " << img.size() << " bytes" << endl << endl;
	}
    // ждём нажатия клавиши
	int key = 0;
	if (wait)
		key = waitKey(0);
	if (!hold)
		destroyWindow(win_name);
	return key;
}

bool file_exist(const string fileName)
{
#ifdef WIN
    std::ifstream infile(fileName);
    return infile.good();
#endif
#ifdef LINUX
	std::ifstream infile(fileName.c_str());
    return infile.good();
	//return true; //КОСТЫЛЬ!
#endif
}

double myceil(const double & x)
{
	if (abs(x - std::floor(x)) < eps)
		return std::floor(x);
	else
		return std::ceil(x);
}

double myfloor(const double & x)
{
	if (abs(x - std::ceil(x)) < eps)
		return std::ceil(x);
	else 
		return std::floor(x);
}

double frac(double x)
{
	if ((abs(x - std::floor(x)) < eps) || (abs(x - std::ceil(x)) < eps))
		return 0;
	else
		return x - std::floor(x);
}

#define TIMING_LOG true
#define TIMING_OUTP true
void timing(const string & s, unsigned int & time, ofstream &log, double * dst = NULL)
{
	double t = ((double) clock() - time) / CLOCKS_PER_SEC;
	if (dst == NULL)
	{
		if (TIMING_LOG)
			log << s << t << " sec." << endl;
		if (TIMING_OUTP)
			cout << s << t << " sec." << endl;
	}
	else
		(*dst) += t;
	time = clock();
}

Mat apply_mask(const Mat & src, const Mat & mask)
{
	Mat res(src.rows, src.cols, CV_8UC1);
	for (int i = 0; i < res.rows; i++)
		for (int j = 0; j < res.cols; j++)
			res.at<uchar>(i, j) = (mask.at<uchar>(i, j) >= 128) ? 255 : src.at<uchar>(i, j);
	return res;
}


void polar_transform(const Mat & src, Mat & dst, const Circ & ib, const Circ & ob, const int frame)
{
	//define dfi ((double) POLAR_W /(2*PI)) // Дискретезация угла.
	int w = POLAR_W;						// По горизонтали - угол.
	int h = POLAR_H;						// По вертикали   - расстояние.
	dst = Mat(h + 2*frame, w + 2*frame + 2*POLAR_CROSS, CV_8UC1);

	dst = SOL_ZERO;

	for (int j = 0; j < dst.cols; j++)
	{
		double fi = (double)(j-frame-POLAR_CROSS) / dfi;
		//double rho00 = sqrt((double) (sqr(ob.c.x-ib.c.x+(ob.R-ib.R)*cos(fi))
		//					  + sqr(ob.c.y-ib.c.y+(ob.R-ib.R)*sin(fi))));
		double rho00 = rho0(ib, ob, fi);
		//test(fi, " Polar transform, rho00");
		for (int i = 0; i < dst.rows; i++)
		{
			int real_i;
			if (i < frame)
				real_i = frame - i;
			else if (i >= h + frame)
				real_i = h - i + (h + frame);
			else
				real_i = i - frame;

			double x = (double) ib.c.x + (ib.R + rho00*real_i/h)*cos(fi);
			double y = (double) ib.c.y + (ib.R + rho00*real_i/h)*sin(fi);
			//test(x, "Polar transform at i = " + IntToStr(i) + ", j = " + IntToStr(j) + ". x:");
			//test(y, "Polar transform at i = " + IntToStr(i) + ", j = " + IntToStr(j) + ". x:");

			Pnt p00(myfloor(x), myfloor(y));
			Pnt p01(myfloor(x), myceil(y));
			Pnt p10(myceil(x), myfloor(y));
			Pnt p11(myceil(x), myceil(y));

			/*//Проверка покрытия:
			p00.draw(dst, COL_GRAY(64*(1-frac(x))*(1-frac(y))), false);
			p01.draw(dst, COL_GRAY(64*(1-frac(x))*(frac(y))), false);
			p10.draw(dst, COL_GRAY(64*(frac(x))*(1-frac(y))), false);
			p11.draw(dst, COL_GRAY(64*(frac(x))*(frac(y))), false);
			*/

			Pnt p(j, i);
			double col = 0;
			if ((x <= src.cols-1) && (y <= src.rows-1))
			{
				col += (1-frac(x))*(1-frac(y)) * p00.GetBright(src);
				col += (1-frac(x))* (frac(y))  * p01.GetBright(src);
				col +=  (frac(x)) *(1-frac(y)) * p10.GetBright(src);
				col +=  (frac(x)) * (frac(y))  * p11.GetBright(src);
			}
			else if (x <= src.cols-1)
			{
				col += (1-frac(x))* p00.GetBright(src);
				col +=  (frac(x)) * p10.GetBright(src);
			}
			else if (y <= src.rows-1)
			{
				col += (1-frac(y)) * p00.GetBright(src);
				col +=  (frac(y))  * p01.GetBright(src);
			}
			else
				col = p00.GetBright(src);
			//test(col, "Polar transform at i = " + IntToStr(i) + ", j = " + IntToStr(j) + ". col:");
			p.draw(dst, COL_GRAY(myfloor(col)));
		}
	}
}

void reverse_polar_transform(const Mat & src, Mat & dst, const Circ & ib, const Circ & ob, const int frame)
{
	int w = POLAR_W;
	int h = POLAR_H;
	int rw = ob.c.x + ob.R+1;
	int rh = ob.c.y + ob.R+1;
	dst = Mat(rh, rw, CV_8UC1);

	dst = SOL_ZERO;

	for (int y = 0; y < rh; y++)
		for (int x = 0; x < rw; x++)
		{
			if (ib.inside(Pnt(x, y)) || !ob.inside(Pnt(x, y)))
				continue;
			double fi = atan2((double) y - ib.c.y, (double) x - ib.c.x);
			if (fi < 0)
				fi = 2*PI + fi;
			//double rho00 = sqrt((double) (sqr(ob.c.x-ib.c.x+(ob.R-ib.R)*cos(fi))
			//				  + sqr(ob.c.y-ib.c.y+(ob.R-ib.R)*sin(fi))));
			double rho00 = rho0(ib, ob, fi);
			double rho = (sqrt((double) (sqr(x - ib.c.x) + sqr(y - ib.c.y))) - ib.R) * POLAR_H / rho00;
			double i = rho + frame;
			double j = fi * dfi + frame + POLAR_CROSS;
			Pnt p00(myfloor(j), myfloor(i));
			Pnt p01(myfloor(j), myceil(i));
			Pnt p10(myceil(j), myfloor(i));
			Pnt p11(myceil(j), myceil(i));

			double col = 0;
			if ((i <= POLAR_H - 1) && (j <= POLAR_W - 1))
			{
				col += (1-frac(j))*(1-frac(i)) * p00.GetBright(src);
				col += (1-frac(j))* (frac(i))  * p01.GetBright(src);
				col +=  (frac(j)) *(1-frac(i)) * p10.GetBright(src);
				col +=  (frac(j)) * (frac(i))  * p11.GetBright(src);
			}
			else if (j <= POLAR_W - 1)
			{
				col += (1-frac(j))* p00.GetBright(src);
				col +=  (frac(j)) * p10.GetBright(src);
			}
			else if (i <= POLAR_H - 1)
			{
				col += (1-frac(i)) * p00.GetBright(src);
				col +=  (frac(i))  * p01.GetBright(src);
			}
			else
				col = p00.GetBright(src);

			dst.at<uchar>(y, x) = myfloor(min(col, 255.0));
		}
}

double calc_nD(const Mat & polar, int x0, int frame) //Для отладки
{
	double cur_sum = 0;
	double cur_sum2 = 0;
	int width_2 = S_WIDTH * POLAR_W/2 / 512 + frame;
	for (int x = x0 - width_2; x < x0 + width_2; x++)
		for (int y = 0; y < POLAR_H + 2*frame; y++)
		{
			uchar t = polar.at<uchar>(y, x);
			cur_sum += t;
			cur_sum2 += t*t;
		}
	int n = width_2 * 2 * POLAR_H;
	return (cur_sum2 - cur_sum * cur_sum / n) * sqr(128.0 * n / cur_sum);
}

double calc_Excess(const Mat & polar, int x0, int frame) //Для отладки
{
	long long cur_sum = 0;
	long long cur_sum2 = 0;
	int width_2 = S_WIDTH * POLAR_W/2 / 512 + frame;
	int n = width_2 * 2 * POLAR_H;
	for (int x = x0 - width_2; x < x0 + width_2; x++)
		for (int y = 0; y < POLAR_H + 2*frame; y++)
		{
			cur_sum += polar.at<uchar>(y, x);
		}

	double avg = ((double) cur_sum) / n;
	cur_sum = 0;

	for (int x = x0 - width_2; x < x0 + width_2; x++)
		for (int y = 0; y < POLAR_H + 2*frame; y++)
		{
			cur_sum += sqr(sqr((long long) polar.at<uchar>(y, x) - avg));
		}
	return cur_sum * sqr(sqr(128.0 / avg));
}

void draw_sector(Mat & polar, const double & s_ang, int frame)
/*	Mat & polar,			//Полярная картинка. На ней рисуется сектор.
	const Circ & ib,		//inner_bound
	const Circ & ob,		//outer_bound
	const double & s_ang	//Серединный угол рисуемого сектора.*/
{
	int w = POLAR_W;
	int h = POLAR_H;
	int sw = w * S_WIDTH / 512;
	int sh = h;

	int Sx0 = w * (s_ang - S_WIDTH/2) / 512 + POLAR_CROSS + frame;
	int Sy0 = frame;
	Rect S(Sx0, Sy0, sw, sh);
	polar(Rect (Sx0 - frame, Sy0 - frame, sw + 2*frame, sh + 2*frame)) /= 1.5;
	polar(S) /= 1.5;
	polar(Rect (w * s_ang / 512 + POLAR_CROSS + frame - 1, Sy0 - frame, 1, sh + 2*frame)) /= 1.5;
}
int get_sector_angle(const Mat & polar, int frame, Mat* plot_img)
{
	long long cur_sum = 0;
	long long cur_sum2 = 0;
	int width_2 = S_WIDTH * POLAR_W/2 / 512 + frame;
	int x0 = POLAR_CROSS + frame; 
	int s_angle = 0;
	int n = width_2 * 2 * (POLAR_H + 2*frame);

	double Dmin = calc_Excess(polar, x0, frame);
	//double Dmin = calc_nD(polar, x0, frame);

	vector <double> plot;
	plot.reserve(polar.cols);
	plot.push_back(Dmin);
	double dcur = 0;
	for (int x = x0+1; x < POLAR_W + POLAR_CROSS; x++)
	{
		//Запрет верхней полуокружности.
		if ((2*(x - x0) >= POLAR_W) && ((x - x0) <= POLAR_W))
		{
			plot.push_back(dcur);
			continue;
		}
	/*	for (int y = 0; y < POLAR_H; y++)
		{
			uchar t = polar.at<uchar>(y, x - width_2 - 1);
			cur_sum -= t;
			cur_sum2 -= ((int)t)*t;
			t = polar.at<uchar>(y, x + width_2 - 1);
			cur_sum += t;
			cur_sum2 += ((int)t)*t;
		}*/
		//long long dcur = cur_sum2 - cur_sum * cur_sum / n;

		dcur = calc_Excess(polar, x, frame);
		//dcur = calc_nD(polar, x, frame);
		plot.push_back(dcur);
		if (Dmin > dcur)
		{
			Dmin = dcur;
			s_angle = (x - x0)*512/POLAR_W;
		}

		/*
		Mat tmp(polar.rows, polar.cols, CV_8UC1);

		for (int i = 0; i < polar.rows; i++)
			for (int j = 0; j < polar.cols; j++)
				tmp.at<uchar>(i, j) = polar.at<uchar>(i, j);

		draw_sector(tmp, (x - x0)*512/POLAR_W, frame);
		IMShow(tmp);
		debug << "x = " << x << ", Dcur = " << dcur << " (Test value = " << calc_nD(polar, x, frame) << "), Dmin = " << Dmin << endl;
		cout << "x = " << x << ", Dcur = " << dcur << " (Test value = " << calc_nD(polar, x, frame) << "), Dmin = " << Dmin << endl;
		//*/
	}
	/*for (int i = 0; i < width_2; i++)
	{
		plot.push_back(-1);
		plotD.push_back(-1);
	}*/
	//Mat polar3b(polar.rows, polar.cols, CV_8UC3);
	//cvtColor(polar, polar3b, CV_GRAY2RGB);
	Mat tmp;
	polar.copyTo(tmp);
	draw_sector(tmp, s_angle, frame);

	(*plot_img) = Concatinate<uchar>(tmp, plot_vector_log<double>(plot,  122, tmp.cols, x0), dir_up);
	/*IMShow(plot_vector_log(plot, 100, COL_WHITE), "Excess", !SHOW_WAIT);
	/*IMShow(plot_vector_log(plotD, 100, COL_WHITE), "D", !SHOW_WAIT);*/
	return s_angle;
}

typedef int (*ProcessFunction) (const string &, ifstream &);
typedef void(*IgnoreFunction) (ifstream &);

#define INPUT_ALL "#ALL"
#define INPUT_SORTED "#SORTED"
#define INPUT_END "#END"
#define INPUT_IGNORE "#-"
int run(const string & data_filename, const string & process_filename, ProcessFunction proc, IgnoreFunction ignore, bool proc_need_images = true)
{
	string imgfilename;
	ifstream data(data_filename.c_str());
	ifstream process_stream(process_filename.c_str());
	vector <string> process_list;
	vector <string> ignore_list;
	bool process_all = false;
	bool input_sorted = false;
	while (process_stream >> imgfilename)
	{
		if (imgfilename[0] != '/')
			if (imgfilename[0] != '#')
				process_list.push_back(imgfilename);
			else
			{
				if (imgfilename == INPUT_ALL)
					process_all = true;
				else if (imgfilename == INPUT_SORTED)
					input_sorted = true;
				else if (imgfilename == INPUT_IGNORE)
				{
					process_stream >> imgfilename;
					ignore_list.push_back(imgfilename);
				}
			}
	}
	if (!input_sorted)
	{
		sort(process_list.begin(), process_list.end());
		sort(ignore_list.begin(), ignore_list.end());
	}

	int k = 0;
	int k_ignore = 0;
	int success = 0;
	while ((data >> imgfilename) && (process_all || k < process_list.size()))
	{
		if (process_all)
		{
			if ((k_ignore < ignore_list.size()) && (imgfilename == ignore_list[k_ignore]))
			{
				k_ignore++;
				ignore(data);
				if (TIMING_TYPE != TIMING_NO)
					cout << "File " << imgfilename << " was ignored." << endl;
				continue;
			}
			else if ((proc_need_images) &&(!file_exist(FOLDER_IMG + imgfilename)))
			{
				if (TIMING_TYPE != TIMING_NO)
					cout << "Error! File " << imgfilename << " doesn't exist." << endl;
				ignore(data);
			}
			else
			{
				if (TIMING_TYPE != TIMING_NO)
					cout << '[' << k << "] ";
				int t = proc(imgfilename, data);
				if (t != 0)
					cout << "Error while proceeding file [" << k << "]: "  << imgfilename << ". Exited with code " << t << endl;
				else
					success++;
				k++;
			}
		}
		else
		{
			if (imgfilename != process_list[k])
			{
				ignore(data);
			}
			else
			{
				if (TIMING_TYPE != TIMING_NO)
					cout << '[' << k << "] ";
				k++;
				if ((k_ignore < ignore_list.size()) && (imgfilename == ignore_list[k_ignore]))
				{
					k_ignore++;
					ignore(data);
					if (TIMING_TYPE != TIMING_NO)
						cout << "File " << imgfilename << " was ignored." << endl;
					continue;
				}
				int t = proc(imgfilename, data);
				if (t != 0)
					cout << "Error while proceeding file [" << k << "]: " << imgfilename << ". Exited with code " << t << endl;
				else
					success++;
			}
		}
	}
	if ((!process_all) && (k != process_list.size()))
		cout << "Input parameters for file [" << k << "]: "  << imgfilename << " don't exist. Process terminated." << endl;
	return success;
}

Mat unshift(const Mat & img, Pnt shift, int cols, int rows)
{
	Mat res(rows, cols, CV_8UC1);
	res = SOL_ZERO;
	for (int i = 0; i < img.cols; i++)
		for (int j = 0; j < img.rows; j++)
			res.at<uchar>(j + shift.y, i + shift.x) = img.at<uchar>(j, i);
	return res;
}

#endif