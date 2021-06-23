#include "Simulatorbase.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <cmath>
#include <float.h>

void DAVISSimulator::resetMemory(int width, int height, int channels)
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
	DVSsave.resize(width * height, 0);
	Lastgray.resize(width * height, 0);
	accumulator.resize(width * height, 0);
}

int DAVISSimulator::SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream)
{

	if(channels == 1)
	{
		SimulateoneChannel(img, outstream);
		SimulateImageBlur(img);
		return 0;
	}
	else
	{
		std::cout << "Error to simulate image with channels " << channels << std::endl
					<< "Please reset the simulator." << std::endl;
		return 1;
	}
}

void DAVISSimulator::SimulateoneChannel(cv::Mat& img, std::ofstream& outstream)
{
	if (frames == 0)
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				uint8_t _gray = img.at<uchar>(j, i);							
				Lastgray[i * height	+ j] = DVSsave[i * height + j] = Clamp(_gray, 0, 255);
			}
		}
		frames++;
		return;
	}

	std::vector<Eventrecord> DVSEvents;
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{		
			int idx = i * height + j;
			int value = Clamp(img.at<uchar>(j, i), 0, 255);

			// DVS part
			int DVSpolar = 0;
			int total_change = value - Lastgray[idx];
			if (value - DVSthres >= DVSsave[idx] || value + DVSthres <= DVSsave[idx])
			{
				DVSpolar = value > DVSsave[idx] ? 1 : -1;
				if (total_change == 0)
				{
					int strange_events = (Lastgray[idx] - DVSsave[idx]) * DVSpolar / DVSthres;
					for (int i = 0; i < strange_events; ++i)
					{
						Eventrecord dev(i, j, frames, DVSpolar);
						DVSEvents.push_back(dev);
						DVSsave[idx] = Lastgray[idx];
					}
				}
				else
				{
					int current_event = DVSsave[idx] + DVSthres * DVSpolar;
					double start_time = (current_event - Lastgray[idx]) / (double)total_change;
					while(start_time >= 0 && start_time <= 1)
					{
						Eventrecord dev(i, j, frames + start_time, DVSpolar);
						DVSEvents.push_back(dev);
						DVSsave[idx] = current_event;
						start_time += DVSthres * DVSpolar / (double)total_change;
						current_event += DVSthres * DVSpolar;
					}
				}
			}
			Lastgray[idx] = value;
		}
	}
	frames++;
	for (const auto dev : DVSEvents)
	{
		outstream << dev.x << " " << dev.y << " " << dev.t << " " << dev.p << ", ";
		D_event++;
	}
}

void DAVISSimulator::SimulateImageBlur(cv::Mat& img)
{
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{		
			int idx = i * height + j;
			int value = Clamp(img.at<uchar>(j, i), 0, 255);
			accumulator[idx] += value;
		}
	}
	if (frames % merge_image_frames == 0)
	{
		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{		
				int idx = i * height + j;
				img.at<uchar>(j, i) = accumulator[idx] / merge_image_frames;
				accumulator[idx] = 0;
			}
		}
		char buffer[256];
		sprintf_s(buffer, 256, file_fmt.c_str(), frames / merge_image_frames);
		cv::imwrite(buffer, img);
	}
}