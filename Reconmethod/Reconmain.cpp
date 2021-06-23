#include "getopt.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <stack>
#include "Reconbase.h"
#include <float.h>

using namespace cv;
using namespace std;

string AERname[5] = { "Retina", "ATIS", "DAVIS", "CeleX", "FSM" };

int main(int argc, char** argv)
{
	string outputdir, inputfile;
	int totalframes = 50;
	int Width = 1280, Height = 720;
	int DVSthres = 25, IVSthres = 2550;
	string outfmt = "out", input_img = "in";
	string first_img = "first";
	double win_size = 9.5;
	int merge_img = 30;
	double max_inte = 40;
	int c;
	int type = -1;
	while (1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{"width",			required_argument, 0, 'w'},
			{"height",			required_argument, 0, 'h'},
			{"frame", 			required_argument, 0, 'f'},
			{"inputdat",		required_argument, 0, 'i'},
			{"outfmt",			required_argument, 0, 'o'},
			{"dvsthres",		required_argument, 0, 'd'},
			{"ivsthres",		required_argument, 0, 't'},
			{"detect_size",		required_argument, 0, 'z'},
			{"davisinput",		required_argument, 0, 'a'},
			{"davismerge",		required_argument, 0, 'm'},
			{"maxinte",			required_argument, 0, 'x'},
			{"type",			required_argument, 0, 'y'},
			{"celexfirst",		required_argument, 0, 'c'},
		};
		c = getopt_long(argc, argv, "w:h:f:i:o:d:t:z:a:m:x:y:c:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
		case 'w':
			Width = atoi(optarg);
			break;
		case 'h':
			Height = atoi(optarg);
			break;
		case 'f':
			totalframes = atoi(optarg);
			break;
		case 'i':
			inputfile = optarg;
			break;
		case 'o':
			outfmt = optarg;
			break;
		case 'd':
			DVSthres = atoi(optarg);
			break;
		case 't':
			IVSthres = atoi(optarg);
			break;
		case 'z':
			win_size = atof(optarg);
			break;
		case 'a':
			input_img = optarg;
			break;
		case 'm':
			merge_img = atoi(optarg);
			break;
		case 'x':
			max_inte = atof(optarg);
			break;
		case 'y':
			type = atoi(optarg);
			break;
		case 'c':
			first_img = optarg;
			break;
		default:
			printf("getopt get undefined character code 0%o\n", c);
			return 0;
			break;
		}
	}
	cout << "Width: " << Width << " Height: " << Height << endl;
	cout << "DVSthres: "  << DVSthres << " IVSthres: " << IVSthres << " detect_size: " << win_size << endl;

	if (Width <= 0 || Height <= 0 || totalframes <= 0)
	{
		cout << "Init image failed." << endl;
		return 0;
	}

	Reconbase* Reconmethod;
	switch (type)
	{
	case 0:
		Reconmethod = new ReconRetina(Width, Height, IVSthres, DVSthres, win_size, outfmt);
		break;
	case 1:
		Reconmethod = new ReconATIS(Width, Height, IVSthres, DVSthres, max_inte, outfmt);
		break;
	case 2:
		Reconmethod = new ReconDAVIS(Width, Height, DVSthres, merge_img, outfmt, input_img);
		break;
	case 3:
		Reconmethod = new ReconCeleX(Width, Height, DVSthres, outfmt, first_img);
		break;
	case 4:
		Reconmethod = new ReconFSM(Width, Height, IVSthres, outfmt);
		break;
	default:
		cout << "Error Type!" << endl;
		return 0;
		break;
	}

	cout << "Running Recon Method of " << AERname[type] << endl;

	ifstream AERdata;
	AERdata.open(inputfile, std::ios::binary);
	Reconmethod->GenerateImages(totalframes, AERdata);

	return 0;
}
