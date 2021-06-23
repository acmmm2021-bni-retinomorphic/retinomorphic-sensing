#include<opencv2/opencv.hpp>
#include<iostream>
#include<fstream>
#include<cstdio>
#include<cmath>
#include "Simulatorbase.h"
#include "parse.h"

using namespace cv;
int brighttrans[256], darktrans[256];
void init_gamma(double gamma);
int main(int argc, char** argv)
{
	parse_command(argc, argv);
	if (type == EMPTY)
	{
		cout << "No input file, please select an image or a directory." << endl;
		show_help();
		return 0;
	}
	//init_gamma(3.3);

	static Mat img;
	if (type == FORMAT)
	{
		/*
		 * simulate spikes of a sequence of images
		 * initalize simulator, continuously load images and simulate
		 */
		int file_idx = START_IDX;
		char buffer[256];
		int symbolnum = 0;
		for (int i = 0; i < file_format.size(); i++)
		{
			if (file_format[i] == '%')
				symbolnum++;
			if (symbolnum >= 2)
			{
				cout << "Too many % in the format, fail to parse." << endl;
				return 0;
			}
		}
		if (symbolnum == 0)
		{
			cout << "Warning: no % in format, simulate may fail." << endl;
		}
		sprintf_s(buffer, 256, file_format.c_str(), file_idx);

		string input_file = buffer;
		img = imread(input_file, 0);
		if (img.empty())
		{
			cout << "Failed to load file: " << input_file << endl;
			return 0;
		}
		if (USEIMAGESIZE)
		{
			WIDTH = img.cols;
			HEIGHT = img.rows;
		}
		std::ofstream info_outstream;
		info_outstream.open("eventsinfo.txt", std::ios::binary | std::ios::app);

		info_outstream << "IVSTHRES: " << IVSTHRES << " DVSTHRES: " << DVSTHRES << endl;
		info_outstream << "WIDTH:" << WIDTH << " HEIGHT:" << HEIGHT << endl;
		info_outstream << "FILE:" << file_format << endl;

		RetinaSimulator Retina_simulator(WIDTH, HEIGHT, CHANNELS, IVSTHRES, DVSTHRES, WIN_SIZE);
		FSMSimulator FSM_simulator(WIDTH, HEIGHT, CHANNELS, IVSTHRES);
		DAVISSimulator DAVIS_simulator(WIDTH, HEIGHT, CHANNELS, DVSTHRES, MERGE_NUM, DAVIS_out);
		ATISSimulator ATIS_simulator(WIDTH, HEIGHT, CHANNELS, IVSTHRES, DVSTHRES, MAX_INTE_TIME);
		CeleXSimulator CeleX_simulator(WIDTH, HEIGHT, CHANNELS, DVSTHRES);
		VidarSimulator Vidar_simulator(WIDTH, HEIGHT, CHANNELS, IVSTHRES, true, LINEAR_FRAMES);

		std::ofstream Retina_outstream;
		Retina_outstream.open("out\\Retina_" + output_filename, std::ios::binary);
		std::ofstream FSM_outstream;
		//FSM_outstream.open("out\\FSM_" + output_filename, std::ios::binary);
		std::ofstream DAVIS_outstream;
		//DAVIS_outstream.open("out\\DAVIS_" + output_filename, std::ios::binary);
		std::ofstream ATIS_outstream;
		//ATIS_outstream.open("out\\ATIS_" + output_filename, std::ios::binary);
		std::ofstream CeleX_outstream;
		//CeleX_outstream.open("out\\CeleX_" + output_filename, std::ios::binary);
		std::ofstream Vidar_outstream;
		//Vidar_outstream.open("out\\Vidar_" + output_filename, std::ios::binary);

		Size ResizeSize = Size(WIDTH, HEIGHT);

		while (1)
		{
			if (!USEIMAGESIZE)
			{
				resize(img, img, ResizeSize);
			}
			Retina_simulator.SimulateEventFromImage(img, Retina_outstream);
			//FSM_simulator.SimulateEventFromImage(img, FSM_outstream);
			//ATIS_simulator.SimulateEventFromImage(img, ATIS_outstream);
			//CeleX_simulator.SimulateEventFromImage(img, CeleX_outstream);
			//Vidar_simulator.SimulateEventFromImage(img, Vidar_outstream);
			//DAVIS_simulator.SimulateEventFromImage(img, DAVIS_outstream);
			//remember DAVIS simulating last, it may change "img"

			// load next image, break when empty
			file_idx++;
			sprintf_s(buffer, 256, file_format.c_str(), file_idx);
			input_file = buffer;
			img = imread(input_file, 0);
			if (img.empty() || file_idx >= END_IDX)
			{
				cout << "Simulate end here." << endl;
				break;
			}

			if (file_idx % 200 == 0)
			{
				cout << file_idx << " frames finished!" << endl;
			}
		}
		Retina_outstream.close();
		Vidar_outstream.close();
		DAVIS_outstream.close();
		ATIS_outstream.close();
		FSM_outstream.close();
		CeleX_outstream.close();

		cout << file_idx - START_IDX << " image(s) is(are) simulated in total." << endl;
		info_outstream << file_idx - START_IDX << " image(s) is(are) simulated in total." << endl;

		info_outstream << "RetinaSimulaor: D-event: " << Retina_simulator.D_event << " I_event: " << Retina_simulator.I_event << endl;
		info_outstream << "ATISSimulaor: D-event: " << ATIS_simulator.D_event << " I_event: " << ATIS_simulator.I_event << endl;
		info_outstream << "FSMSimulaor: D-event: " << FSM_simulator.D_event << " I_event: " << FSM_simulator.I_event << endl;
		info_outstream << "DAVISSimulaor: D-event: " << DAVIS_simulator.D_event << " I_event: " << DAVIS_simulator.I_event << endl;
		info_outstream << "CeleXSimulaor: D-event: " << CeleX_simulator.D_event << " I_event: " << CeleX_simulator.I_event << endl;
		info_outstream << "VidarSimulaor: D-event: " << Vidar_simulator.D_event << " I_event: " << Vidar_simulator.I_event << endl;
		info_outstream.close();

		return 0;
	}

	cout << "Something strange occurs......" << endl;
	return 0;
}

void init_gamma(double gamma)
{
	double b, d;
	if (gamma < 1)
	{
		b = gamma;
		d = 1 / gamma;
	}
	else
	{
		b = 1 / gamma;
		d = gamma;
	}
	for (int i = 0; i < 256; i++)
	{
		brighttrans[i] = pow((i + 0.5) / 256, b) * 256 - 0.5;
		darktrans[i] = pow((i + 0.5) / 256, d) * 256 - 0.5;
	}
}