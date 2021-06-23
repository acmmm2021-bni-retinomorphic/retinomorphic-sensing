#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
#include "bitbuffer.h"

class Simulatorbase{

protected:
	/* defination of image */
	int width;
	int height;
	int channels;					//assert 1

	/* defination of simulator */
	double IVSthres;
	int DVSthres;

	int frames;	// use for initiating Lastgray
	std::vector<int> Lastgray;

	/*
	 * simulate different types of image, you can set your own here
	 * defination outside the class will not make it inline
	 */
	virtual void SimulateoneChannel(cv::Mat& img, std::ofstream& outstream) {
		std::cout << "This should be a virtual class!" << std::endl;
	}
	inline int Clamp(int target, int min, int max);
	inline int clamp(int target);
	inline double find_result(double x1, double x2, double y1, double y2, double ans);
public:
	long long int D_event, I_event;
	Simulatorbase(int _width = 400, int _height = 250, int _channels = 1, double _It = 1020, int _Dt = 50):
	width(_width), height(_height), channels(_channels), IVSthres(_It), DVSthres(_Dt), frames(0)
	{
		// resetMemory(width, height, channels);
		D_event = I_event = 0;
	}
	virtual void resetMemory(int width, int height, int channels) {
		std::cout << "This should be a virtual class!" << std::endl;
	}
	virtual int SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream) {
		std::cout << "This should be a virtual class!" << std::endl;
		return -1;
	}
};
struct Eventrecord
{
	int x;
	int y;
	double t;
	int p;
	bool operator <(const Eventrecord& Er) const
	{
		return t < Er.t;
	}
	Eventrecord(int _x, int _y, double _t, int _p):
	x(_x), y(_y), t(_t), p(_p) {}
};

struct CeleXrecord
{
	int x;
	int y;
	double t;
	int p;
	int val;
	bool operator <(const CeleXrecord& Cr) const
	{
		return t < Cr.t;
	}
	CeleXrecord(int _x, int _y, double _t, int _p, int _val):
	x(_x), y(_y), t(_t), p(_p), val(_val) {}
};

inline double Simulatorbase::find_result(double x1, double x2, double y1, double y2, double ans)
{
	double eps = 1e-6;
	if (ans	< 0)
		return 0;
	double delta_x = x2 - x1;
	double max_inte = delta_x * (y1 + y2) / 2;
	
	// no solution
	if(max_inte + eps < ans)
		return -1;

	// y = b
	if(fabs(y1 - y2) < eps && fabs(y1) > eps)
	{
		return ans / y1 + x1;
	}
	
	// y = kx + b
	double k = (y2 - y1) / delta_x;
	double b = y1 - k * x1;
	// kt^2 + 2bt - (kx1^2 + 2bx1 + 2 * ans) = 0
	double c = (k * x1 + 2 * b) * x1 + 2 * ans;

	if(b*b + k*c < eps || fabs(k) < eps)
	{
		return -1;
	}

	double res1 = (-b + sqrt(b*b + k*c)) / k;
	double res2 = (-b - sqrt(b*b + k*c)) / k;
	
	if(res1 >= x1 && res1 <= x2 && _finite(res1))
		return res1;
	else if(res2 >= x1 && res2 <= x2 && _finite(res2))
		return res2;
	else return -1;
}

inline int Simulatorbase::Clamp(int target, int min, int max)
{
	if(target < min)
		return min;
	else if(target > max)
		return max;
	else return target;
}
inline int Simulatorbase::clamp(int target)
{
	return Clamp(target, 0, 255);
}

class FSMSimulator : public Simulatorbase
{
private:
	std::vector<double> accumulator;
	virtual void SimulateoneChannel(cv::Mat& img, std::ofstream& outstream);
public:
	FSMSimulator(int _width = 400, int _height = 250, int _channels = 1, double _It = 1020) :
		Simulatorbase(_width, _height, _channels, _It, 0)
	{
		resetMemory(width, height, channels);
	}
	virtual void resetMemory(int width, int height, int channels);
	virtual int SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream);
};

class ATISSimulator : public Simulatorbase
{
private:
	std::vector<double> accumulator;
	std::vector<int> DVSsave;
	std::vector<double> Reftime;
	double max_inte_time;
	virtual void SimulateoneChannel(cv::Mat& img, std::ofstream& outstream);
public:
	ATISSimulator(int _width = 400, int _height = 250, int _channels = 1, double _It = 1020, double _Dt = 25, double _mit = 40) :
		Simulatorbase(_width, _height, _channels, _It, _Dt), max_inte_time(_mit)
	{
		resetMemory(width, height, channels);
	}
	virtual void resetMemory(int width, int height, int channels);
	virtual int SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream);
};

class DAVISSimulator : public Simulatorbase
{
private:
	std::vector<double> accumulator;	//use for merging images
	std::vector<int> DVSsave;
	int merge_image_frames;
	std::string file_fmt;
	virtual void SimulateoneChannel(cv::Mat& img, std::ofstream& outstream);
	void SimulateImageBlur(cv::Mat& img);
public:
	DAVISSimulator(int _width = 400, int _height = 250, int _channels = 1, double _Dt = 25, int _mif = 40, std::string _ff = "") :
		Simulatorbase(_width, _height, _channels, 0, _Dt), merge_image_frames(_mif), file_fmt(_ff)
	{
		resetMemory(width, height, channels);
	}
	virtual void resetMemory(int width, int height, int channels);
	virtual int SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream);
};

class CeleXSimulator : public Simulatorbase
{
private:
	std::vector<int> DVSsave;
	virtual void SimulateoneChannel(cv::Mat& img, std::ofstream& outstream);
public:
	CeleXSimulator(int _width = 400, int _height = 250, int _channels = 1, double _Dt = 25) :
		Simulatorbase(_width, _height, _channels, 0, _Dt)
	{
		resetMemory(width, height, channels);
	}
	virtual void resetMemory(int width, int height, int channels);
	virtual int SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream);
};

class VidarSimulator : public Simulatorbase
{
private:
	bool keepResidual;
	virtual void SimulateoneChannel(cv::Mat& img, std::ofstream& outstream);
	std::vector<double> accumulator;
	int linear_frames;
public:
	VidarSimulator(int _width = 400, int _height = 250, int _channels = 1, double _It = 1020, bool _keep = true, int _lf = 0) :
		Simulatorbase(_width, _height, _channels, _It, 0), keepResidual(_keep), linear_frames(_lf)
	{
		resetMemory(width, height, channels);
	}
	virtual void resetMemory(int width, int height, int channels);
	virtual int SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream);
};

class RetinaSimulator : public Simulatorbase
{
private:
	double detect_wsize;

	std::vector<double> Accumulator_b, Accumulator_d;	//accumulators
	std::vector<double> Reftime;
	std::vector<int> DVSsave;
	virtual void SimulateoneChannel(cv::Mat& img, std::ofstream& outstream);
public:
	RetinaSimulator(int _width = 400, int _height = 250, int _channels = 1, double _It = 1020, int _Dt = 50, double _dws = 2) :
		Simulatorbase(_width, _height, _channels, _It, _Dt), detect_wsize(_dws)
	{
		if (IVSthres < detect_wsize * 255)
		{
			std::cout << "detect_wsize too large\n";
			detect_wsize = IVSthres / 510;
		}
		resetMemory(width, height, channels);
	}
	virtual void resetMemory(int width, int height, int channels);
	virtual int SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream);
};