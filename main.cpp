/** \mainpage Welcome to the EES localization algorithm page
* 
*  \section Features
* Method localizes eyelids, eyelashes and shadows on iris images. You can try it <a href = ./index2.html> here </a>
* \section S1 For further documentation take a look at "Files" section.
* Download the project from <a href = 123> github </a>
*/



#pragma comment(linker, "/STACK:154857600")

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

#include "SOL_CONST.h"
#include "SOL_classes.h"
#include "SOL_simplemat.h"
#include "SOL_functions.h"
#include "SOL_parameters.h"
#include "SOL_morphology.h"
#include "SOL_report.h"

using namespace std;
using namespace cv;

//#define DEBUG

//#define S_WIDTH 40 //Ширина сектора в отсчётах. Отсчёт - 1/512 окружности.
//true - сортировка для входных даных не требуется.
//#define WIN

//#define PARAM_P 0.85
#define PARAM_P 0.95
//#define DEBUG
#define INNER_BOUNDARY_SHIFT 5 //Увеличение внутреннего радиуса при обучении для избежания чёрных полос.
//ofstream debug("debug_output.txt");
ofstream time_log("time_log.txt");
ofstream E1_stream("E1.txt");
ofstream E2_stream("E2.txt");


//Константы, заадющие классификатор:
#define PARAM_K 1+1+1+5+4		 //Размерность пространства параметров.
#define PARAM_FUNCS 5
double paramtiming[PARAM_FUNCS] = {0, 0, 0, 0, 0};
const ParamCalcFunction param_funcs[PARAM_FUNCS] = {&B, &AvgB, &SigmaB, &DCT5, &Markov};

const string param_func_names[PARAM_FUNCS] =
{
	 "      Param #0 Calc.(B(x,y)): "
	,"      Param #1 Calc.(AvgB):   "
	,"      Param #2 Calc.(SigmaB): "
	,"      Param #3-7 Calc.(DCT):  "
	,"      Param #8-11 Calc.(Mrk): "
};

int frame;
SimpleFMat* lazyParams[POLAR_H + 2*FRAME][POLAR_W + 2*FRAME + 2*POLAR_CROSS];
bool lazyCalculated[POLAR_H + 2*FRAME][POLAR_W + 2*FRAME + 2*POLAR_CROSS];

SimpleFMat CalcParamVector(const Mat & img, Pnt & p)
{
	//SimpleFMat res(PARAM_K, 1);
	//if (!lazyCalculated[p.y][p.x])
	//{
		SimpleFMat res(PARAM_K, 1);
		//lazyCalculated[p.y][p.x] = true;
		double* cur_res = res[0];
		unsigned int time = clock();
		//Можно ускорить, переписав на итераторы:
		for (int i = 0; i < PARAM_FUNCS; i++)
		{
			cur_res += param_funcs[i](img, p, cur_res, cur_res - 1);
			if (TIMING_TYPE == TIMING_MAXIMAL)
				timing(param_func_names[i], time, time_log, paramtiming + i);		
		}
		return res;
	//}
	//return (* lazyParams[p.y][p.x]);
}


//Подавать на вход преобразованный expert, обрезанную по радужке decart_mask.
double calc_error(const Mat & decart_mask, Expert & expert, Circ &ib, Circ &ob)
{
	int E1 = 0;
	int N1 = 0;
	int N2 = 0;
	int E2 = 0;

	for (int i = 0; i < decart_mask.rows; i++)
		for (int j = 0; j < decart_mask.cols; j++)
		{
			Pnt p(j, i);
			if (!ib.inside(p) && ob.inside(p))
				if (expert.point_is_inside(p))
				{
					N1++;
					if (decart_mask.at<uchar>(i, j) < 128)
						E1++;
				}
				else
				{
					N2++;
					if (decart_mask.at<uchar>(i, j) >= 128)
						E2++;
				}
		}
	/*cout << "N1 = " << N1 << "; E1 = " << E1 << "; N2 = " << N2 << "; E2 = " << E2 << endl;
	debug << "N1 = " << N1 << "; E1 = " << E1 << "; N2 = " << N2 << "; E2 = " << E2 << endl;
	debug << (double) E1/N1 + (double) E2/N2 << endl << endl;*/
	E1_stream << ((double) E1)/N1 << " ";
	E2_stream << ((double) E2)/N2 << " ";
	return (double) E1/N1 + (double) E2/N2;
}

double err_sum;
ofstream interest("interest.txt");
ofstream errors("errors.txt");


void postprocess(const Mat & iris, int frame, Mat & mask, Mat & mask_morph)
{
	closing(mask, 3);
	opening(mask, 3);
	mask.copyTo(mask_morph);

	component_lacunas_detection(iris, mask, frame);
	opening(mask, 3);
	closing(mask, 3);
}

void inner_boundary_unshift(Mat & mask, int shift, Circ ib, Circ ob)
{

}

string intToBinStr(int x)
{
	string res = "";
	while (x)
	{
		res += '0' + (x%2 == 1);
		x/=2;
	}
	while (res.length() < 5)
		res += '0';
	return res;
}

int process(const string & imgfilename, ifstream & data)
{
	debug << imgfilename << endl;
	time_log << imgfilename << ": ";
	if (TIMING_TYPE > TIMING_MINIMAL)
		time_log << endl;
	if (TIMING_TYPE != TIMING_NO)
		cout << imgfilename << ": ";
	if (TIMING_TYPE > TIMING_MINIMAL)
		cout << endl;
	unsigned int time = clock();
	unsigned int time_start = time;

	memset(lazyCalculated, false, sizeof(lazyCalculated));
	SOL_report report;
	report.AddHeader(imgfilename + ":");
	//report.AddText(imgfilename + ":", 2);
	report.NewSection();

	//ЧТЕНИЕ ДАННЫХ:

	// получаем картинку
	Mat img0(imread(FOLDER_IMG + imgfilename));
	// обесцвечиваем:
	Mat gray(img0.cols, img0.rows, CV_8UC1);
	cvtColor(img0, gray, CV_RGB2GRAY);

	Circ inner_boundary;
	Circ outer_boundary; 
	Expert expert;
	int s_angle;
	data >> inner_boundary >> outer_boundary >> expert /*>> s_angle*/;
	
	if (TIMING_TYPE >= TIMING_FULL)
		timing("  Data reading:               ", time, time_log);
	if ((outer_boundary.c.x - outer_boundary.R < 0) || (outer_boundary.c.x + outer_boundary.R >= img0.cols)
	     || (outer_boundary.c.y - outer_boundary.R < 0) || (outer_boundary.c.y + outer_boundary.R >= img0.rows))
	{
		debug << "Error! Bad Image: " << imgfilename << endl;
		return 1;
	}

	inner_boundary.draw(img0, COL_GREEN);
	outer_boundary.draw(img0, COL_GREEN);
	//expert.draw(img0, COL_RED);

	//НОРМАЛИЗАЦИЯ:

	Rect iris_rect(outer_boundary.c.x - outer_boundary.R - 1,
			  outer_boundary.c.y - outer_boundary.R - 1,
			  2*outer_boundary.R + 2,
			  2*outer_boundary.R + 2);

	// Задаёт линейное преобразование - перенос начала картинки в угол радужки:
	Pnt shift(outer_boundary.c.x - outer_boundary.R - 1, outer_boundary.c.y - outer_boundary.R - 1);
	Mat iris_decart = gray(iris_rect);

	report.AddImg(iris_decart, "Source:");
	report.NewColumn();

	inner_boundary.move(shift);
	outer_boundary.move(shift);
	expert.move(shift);


	int frame = FRAME;
	Mat iris;

	inner_boundary.R+=INNER_BOUNDARY_SHIFT;
	polar_transform(iris_decart, iris, inner_boundary, outer_boundary, frame);

	report.AddImg(iris, "Polar transform:");
	Mat plot;
	s_angle = get_sector_angle(iris, frame, &plot);
	report.AddImg(plot, "Excess plot:");

	if (TIMING_TYPE >= TIMING_FULL)
		timing("  Data transforming:          ", time, time_log);

	int w = POLAR_W;
	int h = POLAR_H;
	int sw = w * S_WIDTH / 512;
	int sh = h;

	int Sx0 = w * (s_angle - S_WIDTH/2) / 512 + POLAR_CROSS + frame;
	int Sy0 = frame;

	//ОБУЧЕНЕИЕ НА СЕКТОРЕ:

	Rect S(Sx0 - frame, Sy0 - frame, sw + 2*frame, sh + 2*frame);
	//debug << Sx0 << " " << Sy0 << frame << endl;
	SimpleFMat params(PARAM_K, sw*sh);
	SimpleFMat avg(PARAM_K, 1);
	avg = 0;
	
	paramtiming[0] = paramtiming[1] = paramtiming[2] = paramtiming[3] = 0;
	int k = 0;
	for (int x = 0; x < sw; x++)
		for (int y = 0; y < sh; y++)
		{
			Pnt p(S.x + x + frame, S.y + y + frame); 
			SimpleFMat tmp = CalcParamVector(iris, p);
			for(int i = 0; i < PARAM_K; i++)
			{
				avg[i][0] += tmp[i][0];
				params[i][k] = tmp[i][0];
			}
			k++;
		}

#ifdef DEBUG
	debug << params;
#endif
	avg /= (sh*sw);
	for(int i = 0; i < PARAM_K; i++)
		for(int j = 0; j < k; j++)
			params[i][j] -= avg[i][0];

	SimpleFMat C_1 = (params * params.t()).inv();

#ifdef DEBUG
	debug << params;
	debug << std::setprecision(7) << C_1;
#endif

	if (TIMING_TYPE >= TIMING_FULL)
		timing("  Classifier building:        ", time, time_log);
	//ПОСТОЕНИЕ ПЕРВИЧНОЙ МАСКИ:
	//inner_boundary.R-=INNER_BOUNDARY_SHIFT;
	//polar_transform(iris_decart, iris, inner_boundary, outer_boundary, frame);

	Mat mask(iris.rows, iris.cols, CV_8UC1);
	Mat mahalanobis(iris.rows, iris.cols, CV_8UC1);

	mask = SOL_ZERO;
	mahalanobis = SOL_ZERO;

	unsigned int time2 = 0;
	double params_calculation = 0;
	double mahalanobis_calculation = 0;
	
	for (int i = 0; i < PARAM_FUNCS; i++)
		paramtiming[i] = 0;
	for (int x = frame + POLAR_CROSS; x < mask.cols - frame - POLAR_CROSS; x++)
		for (int y = frame; y < mask.rows - frame; y++)
		{
			time2 = clock();
			Pnt p(x, y);
			SimpleFMat xm = (CalcParamVector(iris, p) - avg);
			params_calculation += ((double) clock() - time2) / CLOCKS_PER_SEC;
			time2 = clock();
			double D_2 = abs((xm.t() * C_1 * xm)[0][0]); //Квадрат расстояния Махаланобиса.
			//double bayes_k = 0.75 + (double) bayes_mask.at<uchar>(y - frame, x - frame) / (4*bayes_max);
			double P = exp(- D_2);// / bayes_k;
			mahalanobis.at<uchar>(y, x) = floor(P * 220);
			#ifdef DEBUG
			//debug << xm.t() << D_2 << " " << P << endl;
			#endif
			if (P > PARAM_P)
				mask.at<uchar>(y, x) = 0;
			else
				mask.at<uchar>(y, x) = 255;
			mahalanobis_calculation += ((double) clock() - time2) / CLOCKS_PER_SEC;
		}
	if (TIMING_TYPE >= TIMING_FULL)
	{
		timing("  Mask building:              ", time, time_log);
		if (TIMING_TYPE == TIMING_MAXIMAL)
		{
			cout << "    Params calculating:       " << params_calculation << " sec." << endl;
			for (int i = 0; i < PARAM_FUNCS; i++)
				cout << param_func_names[i] << paramtiming[i] << " sec." << endl;
			cout << "    Mahalanobis calc. :       " << mahalanobis_calculation << " sec." << endl;
		}
	}

	//ПОСТОБРАБОТКА 1:
	report.AddImg(mahalanobis, "Mahalanobis:");
	report.NewColumn();
	report.AddImg(mask, "Pure mask:");

	Mat mask_morph;
	postprocess(iris, frame, mask, mask_morph);
	report.AddImg(mask_morph, "Morphology:");
	report.AddImg(mask, "Mask:");
	/*	
	Mat mask_learning;
	Mat ker(PARAM_A, PARAM_A, CV_8UC1);
	ker = 1;
	dilate(mask, mask_learning, ker);
	//ОБУЧЕНИЕ2.1:
	int N = 0;
	for (int x = frame + POLAR_CROSS; x < mask_learning.cols - frame - POLAR_CROSS; x++)
		for (int y = frame; y < mask_learning.rows - frame; y++)
			if (mask_learning.at<uchar>(y, x) < 128)
				N++;
	SimpleFMat params2(PARAM_K, N);
	SimpleFMat avg1(PARAM_K, 1);
	avg1 = 0;
	k = 0;
	for (int x = frame + POLAR_CROSS; x < mask_learning.cols - frame - POLAR_CROSS; x++)
		for (int y = frame; y < mask_learning.rows - frame; y++)
		{
			if (mask_learning.at<uchar>(y, x) >= 128)
				continue;
			if (k == N)
				continue;
			Pnt p(x, y); 
			SimpleFMat tmp = CalcParamVector(iris, p);
			for(int i = 0; i < PARAM_K; i++)
			{
				avg1[i][0] += tmp[i][0];
				params2[i][k] = tmp[i][0];
			}
			k++;
		}

	avg1 /= N;
	for(int i = 0; i < PARAM_K; i++)
		for(int j = 0; j < k; j++)
			params2[i][j] -= avg1[i][0];
	C_1 = (params2 * params2.t()).inv();

	//ОБУЧЕНИЕ2.2:
	ker = Mat(2, 2, CV_8UC1);
	ker = 1;
	erode(mask, mask_learning, ker);
	//IMShow(mask_learning);

	N = 0;
	for (int x = frame + POLAR_CROSS; x < mask_learning.cols - frame - POLAR_CROSS; x++)
		for (int y = frame; y < mask_learning.rows - frame; y++)
			if (mask_learning.at<uchar>(y, x) < 128)
				N++;
	params2 = SimpleFMat(PARAM_K, N);
	SimpleFMat avg2(PARAM_K, 1);
	avg2 = 0;
	k = 0;
	for (int x = frame + POLAR_CROSS; x < mask_learning.cols - frame - POLAR_CROSS; x++)
		for (int y = frame; y < mask_learning.rows - frame; y++)
		{
			if (mask_learning.at<uchar>(y, x) < 128)
				continue;
			if (k == N)
				continue;
			Pnt p(x, y); 
			SimpleFMat tmp = CalcParamVector(iris, p);
			for(int i = 0; i < PARAM_K; i++)
			{
				avg2[i][0] += tmp[i][0];
				params2[i][k] = tmp[i][0];
			}
			k++;
		}

	avg2 /= N;
	for(int i = 0; i < PARAM_K; i++)
		for(int j = 0; j < k; j++)
			params2[i][j] -= avg2[i][0];
	SimpleFMat C_2 = (params2 * params2.t()).inv();

	//МАСКА2:
	//Mat mahalanobis2(iris.rows, iris.cols, CV_8UC1);
	for (int x = frame + POLAR_CROSS; x < mask.cols - frame - POLAR_CROSS; x++)
		for (int y = frame; y < mask.rows - frame; y++)
		{
			Pnt p(x, y);
			SimpleFMat xm = CalcParamVector(iris, p);
			SimpleFMat xm1 = CalcParamVector(iris, p) - avg1;
			SimpleFMat xm2 = CalcParamVector(iris, p) - avg2;
			double D_2 = abs((xm1.t() * C_1 * xm1)[0][0]); //Квадрат расстояния Махаланобиса.
			double P1 = exp(- D_2);
			D_2 = abs((xm2.t() * C_2 * xm2)[0][0]); //Квадрат расстояния Махаланобиса.
			double P2 = exp(- D_2);
			//mahalanobis2.at<uchar>(y, x) = floor(P * 220);
			//if (P > PARAM_P && mask.at<uchar>(y, x) < 128)
			if (P1 > P2)
				mask.at<uchar>(y, x) = 0;
			else
				mask.at<uchar>(y, x) = 255;
			//mahalanobis_calculation += ((double) clock() - time2) / CLOCKS_PER_SEC;
		}
	Mat mask_morph2;
	postprocess(iris, frame, mask, mask_morph2);
	//*/
	//ВЫВОД:

	if (TIMING_TYPE >= TIMING_FULL)
		timing("  Morphology:                 ", time, time_log);

	draw_sector(iris, s_angle, frame);

	Mat decart_mask(img0.rows, img0.cols, CV_8UC1);
	reverse_polar_transform(mask, decart_mask, inner_boundary, outer_boundary, frame);
	//inner_boundary_unshift(mask, INNER_BOUNDARY_SHIFT, inner_boundary, outer_boundary);
	double err = calc_error(decart_mask, expert, inner_boundary, outer_boundary);
	cout << err;
	
	time_log << "ERROR = " << err << ". ";
	if (TIMING_TYPE >= TIMING_FULL)
		timing("  Error calculation:          ", time, time_log);
	if (TIMING_TYPE >= TIMING_FULL)
		timing("Total:  ", time_start, time_log);
	else if (TIMING_TYPE != TIMING_NO)
		timing("", time_start, time_log);
	if (TIMING_TYPE > TIMING_MINIMAL)
		cout << endl;
	if (TIMING_TYPE != TIMING_NO)
		cout << "ERROR = " << err << endl;

	inner_boundary.draw(iris_decart);
	outer_boundary.draw(iris_decart);
	//expert.draw(iris_decart);
	Mat Result = apply_mask(iris_decart, decart_mask);
	imwrite("Res.jpg", Result);
	report.AddImg(apply_mask(iris, mask), "Applied mask: ");
	report.NewColumn();
	report.AddImg(Result, "Result: ");
	/*IMShow(iris_decart, "Decart", !SHOW_WAIT);
	IMShow(iris, "Polar", !SHOW_WAIT);
	IMShow(plot, "Plot", !SHOW_WAIT);
	IMShow(mahalanobis, "Mahalanobis", !SHOW_WAIT);
	IMShow(mask_pure, "Pure mask", !SHOW_WAIT);
	IMShow(mask_morph, "Morphology", !SHOW_WAIT);
	IMShow(mask, "Lacunas detection", !SHOW_WAIT);
	IMShow(apply_mask(iris, mask), "Applied mask", !SHOW_WAIT);
	IMShow(Result, "Result", !SHOW_WAIT);*/

	//НЕАКТУАЛЬНО:
	//report.NewSection();
//	report.AddEmpty();
	//report.NewColumn();
	//report.AddText("Error = " + ToStr(err), 2);
	//report.NewColumn();
	//report.AddText((string) "Image base: " + BASE_NAME, 1, COL_WHITE);
	report.AddHeader("Error = " + ToStr(err), "", (string) "Image base: " + BASE_NAME);
	Mat report_img = report.generate();
	/*Mat report_img = MakeReport(imgfilename, iris_decart0, iris0, plot, mahalanobis,
		mask_pure, mask_morph, mask, apply_mask(iris, mask), Result, err);*/
	//expert.draw(Result, COL_GRAY(0), 2);
	//IMShow(Result);
	//imwrite("Res.bmp", Result);
	//imwrite("Src.bmp", iris_decart0);
	IMShow(report_img, "Report", SHOW_WAIT);
	String filename0;
	filename0.assign(imgfilename, 0, imgfilename.find('.'));
	imwrite("./Result/" + filename0 + "_occlusion.bmp", Result);
	imwrite("./Mask/" + filename0 + "_occlusion.bmp", unshift(decart_mask, shift, img0.cols, img0.rows));
//	imwrite("./Report/Rep_" + imgfilename, report);
	if (err >= 0.4)
	{
		interest << imgfilename << " //ERR=" << err << endl;
		if (err < 0.5)
			imwrite("./Report/0.4/" + filename0 + "_Rep.bmp", report_img);
		else if (err < 0.6)
			imwrite("./Report/0.5/" + filename0 + "_Rep.bmp", report_img);
		else if (err < 0.7)
			imwrite("./Report/0.6/" + filename0 + "_Rep.bmp", report_img);
		else if (err < 0.8)
			imwrite("./Report/0.7/" + filename0 + "_Rep.bmp", report_img);
		else if (err < 0.9)
			imwrite("./Report/0.8/" + filename0 + "_Rep.bmp", report_img);
		else if (err < 1)
			imwrite("./Report/0.9/" + filename0 + "_Rep.bmp", report_img);
		else
			imwrite("./Report/1.0/" + filename0 + "_Rep.bmp", report_img);
	}
	else
		imwrite("./Report/OK/" + filename0 + "_Rep.bmp", report_img);

	err_sum += err;
	errors << err << ' ';
	return 0;
}

void ignore(ifstream &data)
{
	int x;
	for (int i = 0; i < 86; i++)
		data >> x;
}

int main(int argc, char* argv[])
{
	// имя картинки задаётся первым параметром
//	string imgfilename = argc >= 2 ? argv[1] : "2014R01.bmp";
//	string datfilename = argc >= 3 ? argv[2] : "data.txt";
	char* processfile = "processing.txt";
	if (argc >= 2)
		processfile = argv[1];

	unsigned int time = clock();
	for (int i = 0; i < POLAR_H + 2*FRAME; i++)
		for (int j = 0; j < POLAR_W + 2*FRAME + 2*POLAR_CROSS; j++)
		{
			lazyParams[i][j] = new SimpleFMat(PARAM_K, 1);
			lazyCalculated[i][j] = false;
		}

	/*ifstream data(DAT_FILE);
	ifstream process_stream(processfile);*/
	int num = run(DAT_FILE, processfile, &process, &ignore); //declared in SOL_functions.h

	errors << endl;
	double E = err_sum / num;

	cout << endl << "Avg error = " << E << endl;
	time_log << endl << "Avg error = " << E << endl;
	timing("Total time: ", time, time_log);

	for (int i = 0; i < POLAR_H + 2*FRAME; i++)
		for (int j = 0; j < POLAR_W + 2*FRAME + 2*POLAR_CROSS; j++)
			free(lazyParams[i][j]);

    return 0;
}
