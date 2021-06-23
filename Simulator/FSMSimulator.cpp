#include "Simulatorbase.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <cmath>
#include <float.h>

void FSMSimulator::resetMemory(int width, int height, int channels)
{
	/*
	 * reset the accumulators to fit a new size of image
	 * caution: all value saved will be erased 
	 */
	if(channels != 1)
	{
		std::cout << "Error to reset with channels" << channels << std::endl;
		return;
	}
	if(width <= 0 || height <= 0)
	{
		std::cout << "Bad width or height" << width << " " << height << std::endl;
		return;
	}

	accumulator.resize(width * height, 0);
	Lastgray.resize(width * height, 0);
}

int FSMSimulator::SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream)
{
	/* 
	 * simulate spikes from a given image
	 * 1. resize the image
	 * 2. add the value of each pixel (accumulator for channel==1 && accumulatorR,G,B for channel==3)
	 * 3. compare with the IVSthres and generate spikes! 
	 */

	if(channels == 1)
	{
		SimulateoneChannel(img, outstream);
		return 0;
	}
	else
	{
		std::cout << "Error to simulate image with channels " << channels << std::endl
					<< "Please reset the simulator." << std::endl;
		return 1;
	}
}

void FSMSimulator::SimulateoneChannel(cv::Mat& img, std::ofstream& outstream)
{
	if (frames == 0)
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				uint8_t _gray = img.at<uchar>(j, i);							
				Lastgray[i * height + j] = Clamp(_gray, 0, 255);
			}
		}
		frames++;
		return;
	}

	std::vector<Eventrecord> FSMEvents;
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{		
			int idx = i * height + j;
			int value = Clamp(img.at<uchar>(j, i), 0, 255);
			double res = find_result(0, 1, Lastgray[idx], value, IVSthres - accumulator[idx]);

			if (res < 0)
			{
				accumulator[idx] += (Lastgray[idx] + value) / 2.0;
			}
			else
			{
				Eventrecord Fev(i, j, frames + res, 1);
				FSMEvents.push_back(Fev);
				double endval = Lastgray[idx] + res * (value - Lastgray[idx]);
				accumulator[idx] = (1 - res) * (endval + value) / 2.0;
			}
			Lastgray[idx] = value;
		}
	}
	frames++;

	for (const auto fev : FSMEvents)
	{
		if (!_finite(fev.t))
		{
			int idx = fev.x * height + fev.y;
			accumulator[idx] = 0;
		}
		else
		{
			outstream << fev.x << " " << fev.y << " " << fev.t << " " << fev.p << ", ";
			I_event++;
		}
	}

}
