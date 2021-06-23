#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <random>

using namespace cv;

class Reconbase
{
protected:
	int width;
	int height;
	double IVSthres;
	int DVSthres;
	std::string output_fmt;
public:
	Reconbase(int _width = 400, int _height = 250, double _It = 1020, int _Dt = 25, std::string _out = "") :
		width(_width), height(_height), IVSthres(_It), DVSthres(_Dt), output_fmt(_out) {}
	virtual void GenerateImages(int totalframes, std::ifstream& Eventfile) {}
};

class ReconFSM : public Reconbase
{
public:
	ReconFSM(int _w = 400, int _h = 250, double _It = 1020, std::string _out = "") :
		Reconbase(_w, _h, _It, 0, _out) {}
	virtual void GenerateImages(int totalframes, std::ifstream& Eventfile);
};

class ReconATIS : public Reconbase
{
private:
	double max_inte_time;
public:
	ReconATIS(int _w = 400, int _h = 250, double _It = 1020, int _Dt = 25, double _mit = 40, std::string _out = "") :
		Reconbase(_w, _h, _It, _Dt, _out), max_inte_time(_mit) {}
	virtual void GenerateImages(int totalframes, std::ifstream& Eventfile);
};

class ReconCeleX : public Reconbase
{
private:
	std::string first_frame;
public:
	ReconCeleX(int _w = 400, int _h = 250, int _Dt = 25, std::string _out = "", std::string _ff = "") :
		Reconbase(_w, _h, 0, _Dt, _out), first_frame(_ff) {}
	virtual void GenerateImages(int totalframes, std::ifstream& Eventfile);
};

class ReconDAVIS : public Reconbase
{
private:
	int merge_image_frames;
	std::string input_fmt;
public:
	ReconDAVIS(int _w = 400, int _h = 250, int _Dt = 25, int _mif = 5, std::string _out = "", std::string _in = "") :
		Reconbase(_w, _h, 0, _Dt, _out), merge_image_frames(_mif), input_fmt(_in) {}
	virtual void GenerateImages(int totalframes, std::ifstream& Eventfile);
};

class ReconRetina : public Reconbase
{
private:
	double window_size;
public:
	ReconRetina(int _w = 400, int _h = 250, double _It = 1020, int _Dt = 25, double _ws = 3.8, std::string _out = "") :
		Reconbase(_w, _h, _It, _Dt, _out), window_size(_ws) {}
	virtual void GenerateImages(int totalframes, std::ifstream& Eventfile);
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

inline int Clamp(int target, int min, int max)
{
	if(target < min)
		return min;
	else if(target > max)
		return max;
	else return target;
}
inline int clamp(int target)
{
	return Clamp(target, 0, 255);
}
