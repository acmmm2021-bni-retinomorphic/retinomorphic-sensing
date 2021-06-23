#include<cstdlib> 
#include "getopt.h"
#include<cstring>
#include<iostream>
#include<iomanip>
using std::string;
using std::setw;
using std::cout;
using std::endl;

extern int HEIGHT = 250;
extern int WIDTH = 400;
extern int CHANNELS = 1;
extern bool USEIMAGESIZE = false;

extern int IVSTHRES = 2550;
extern int DVSTHRES = 25;

extern int START_IDX = 1;
extern int END_IDX = 500;

enum filetype{FORMAT, EMPTY};
extern filetype type = EMPTY;
extern string file_format = "";
extern string output_filename = "output.dat";

extern double WIN_SIZE = 9.5;

extern double BASENUMBER = 2;
extern int POWERMAX = 3;

extern int MERGE_NUM = 20;
extern int MAX_INTE_TIME = 40;
extern string DAVIS_out = "out\\DAVIS\\merge%06d.png";

extern int LINEAR_FRAMES = 10;
void show_help();
void parse_command(int argc, char** argv);

void parse_command(int argc, char** argv)
{
	/*
	 * parse command including:
	 * height && width && channels
	 * querytimes && threshold && whether keep residuals
	 *
	 * you can simulate spikes from a single image
	 * or select a folder with a list of images and point the format of the images
	 * maybe a video in the future
	 */
	int c;

	while (1)
	{
		int option_index = 0;
		static struct option long_options[] = {
			{"width", 			required_argument, 0, 'w'},
			{"height", 			required_argument, 0, 'h'},
			{"channels",  		required_argument, 0, 'c'},
			{"endindex",  		required_argument, 0, 'e'},
			{"ivsthres",  		required_argument, 0, 'i'},
			{"startindex",  	required_argument, 0, 's'},
			{"useimgsize",		no_argument,	   0, 'u'},
			{"help",  			no_argument,	   0, '?'},
			{"dvsthres",		required_argument, 0, 'd'},
			{"input_format", 	required_argument, 0, 'f'},
			{"output_file", 	required_argument, 0, 'o'},
			{"detect_size", 	required_argument, 0, 'z'},
			{"basenum", 		required_argument, 0, 'b'},
			{"powermax",		required_argument, 0, 'p'},
			{"linear",			required_argument, 0, 'l'},
			{"mergename",		required_argument, 0, 'm'},
		};

		c = getopt_long(argc, argv, "w:h:c:ue:i:f:o:s:z:d:?b:p:l:m:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
		case 'w':
			WIDTH = atoi(optarg);
			break;
		case 'h':
			HEIGHT = atoi(optarg);
			break;
		case 'c':
			CHANNELS = atoi(optarg);
			break;
		case 'u':
			USEIMAGESIZE = true;
			break;
		case 'i':
			IVSTHRES = atoi(optarg);
			break;
		case 'e':
			END_IDX = atoi(optarg);
			break;
		case 'd':
			DVSTHRES = atoi(optarg);
			break;
		case 'f':
			file_format = optarg;
			type = FORMAT;
			break;
		case 's':
			START_IDX = atoi(optarg);
			break;
		case 'o':
			output_filename = optarg;
			break;
		case 'z':
			WIN_SIZE = atof(optarg);
			break;
		case 'b':
			BASENUMBER = atof(optarg);
			break;
		case 'p':
			POWERMAX = atoi(optarg);
			break;
		case 'l':
			LINEAR_FRAMES = atoi(optarg);
			break;
		case 'm':
			DAVIS_out = optarg;
			break;
		case '?':
			show_help();
			exit(0);
			break;
		default:
			printf("getopt get undefined character code 0%o\n", c);
			show_help();
			exit(1);
			break;
		}
	}

	if (optind < argc) {
		cout << "not parsed arguments: " << endl;
		while (optind < argc)
			cout << argv[optind++] << endl;
	}
}

void show_help()
{
	/*
	 * help function
	 */

	cout << "Usage of spike simulator: " << endl
		<< "-w or --width:\t\tthe width of the image(s)." << endl
		<< "-h or --height:\t\tthe height of the image(s)." << endl
		<< "-c or --channels:\tthe channels of the image(s)." << endl
		<< "-u or --useimgsize:\tuse the w*h of the images(s)" << endl
		<< "-i or --ivsthres:\tthe IVS threshold of the simulator." << endl
		<< "-d or --dvsthres:\tthe DVS threshold of the simulator." << endl
		<< "-z or --detect_size:\tthe detect size of the simulator." << endl
		<< "-f or --input_format:\tformat of a series of input images." << endl
		<< "-s or --startindex:\tthe start index of the images." << endl
		<< "-e or --endindex:\tthe end index of the images." << endl
		<< "-o or --output_file:\tname of output file." << endl
		<< "-? or --help:\t\tthis usage." << endl << endl
		<< "It's necessary to select -f as input." << endl
		<< "It's not for other options, they have default arguments." << endl
		<< "If you choose -u for -f, the width and height depend on the first image" << endl;
}