#include "Simulatorbase.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <cmath>
#include <float.h>

void RetinaSimulator::resetMemory(int width, int height, int channels)
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
	Accumulator_b.resize(width * height, 0);
	Accumulator_d.resize(width * height, 0);
	Reftime.resize(width * height, 0);
	DVSsave.resize(width * height, 0);
	Lastgray.resize(width * height, 0);
}

int RetinaSimulator::SimulateEventFromImage(cv::Mat& img, std::ofstream& outstream)
{

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

void RetinaSimulator::SimulateoneChannel(cv::Mat& img, std::ofstream& outstream)
{
	//static int frames = 0;
	if (frames == 0)
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				uint8_t _gray = img.at<uchar>(j, i);							
				Lastgray[i * height	+ j] = DVSsave[i * height + j] = Clamp(_gray, 0, 255);
				Reftime[i * height + j] = 0;
				Accumulator_b[i * height + j] = Accumulator_d[i * height + j] = 0;
			}
		}
		frames++;
		return;
	}
	std::vector<Eventrecord> HybridEvents;
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{		
			int idx = i * height + j;
			int value = Clamp(img.at<uchar>(j, i), 0, 255);

			// IVS part
			double ans1 = find_result(0, 1, Lastgray[idx], value, IVSthres - Accumulator_b[idx]);
			double ans2 = find_result(0, 1, 255 - Lastgray[idx], 255 - value, IVSthres - Accumulator_d[idx]);
			double ans = ans1;
			int IVSpolar = 1;
			if (ans < 0 || (ans2 < ans && ans2 >= 0))
			{
				ans = ans2;
				IVSpolar = 0;
			}

			// DVS part
			int DVSpolar = 0;
			std::vector<double> DVS_times;
			int total_change = value - Lastgray[idx];
			if (value - DVSthres >= DVSsave[idx] || value + DVSthres <= DVSsave[idx])
			{
				DVSpolar = value > DVSsave[idx] ? 1 : -1;
				if (total_change == 0)
				{
					int strange_events = (Lastgray[idx] - DVSsave[idx]) * DVSpolar / DVSthres;
					for (int i = 0; i < strange_events; ++i)
					{
						DVS_times.push_back(0);
					}
				}
				else
				{
					int current_event = DVSsave[idx] + DVSthres * DVSpolar;
					double start_time = (current_event - Lastgray[idx]) / (double)total_change;
					while(start_time >= 0 && start_time <= 1)
					{
						DVS_times.push_back(start_time);
						start_time += DVSthres * DVSpolar / (double)total_change;
					}
				}
			}

			// IVS failed and DVS failed
			if (ans < 0 && DVSpolar == 0)
			{
				// integrating
				Accumulator_b[idx] += (Lastgray[idx] + value) / 2.0;
				Accumulator_d[idx] += 255 - (Lastgray[idx] + value) / 2.0;
			}
			// IVS succeeded and DVS failed
			if (ans >= 0 && DVSpolar == 0)
			{
				Eventrecord Iev(i, j, frames + ans, IVSpolar);
				HybridEvents.push_back(Iev);
				Reftime[idx] = frames + ans;
				I_event++;
				double end_val = Lastgray[idx] + ans * total_change;
				Accumulator_b[idx] = (end_val + value) * (1 - ans) / 2.0;
				Accumulator_d[idx] = (510 - end_val - value) * (1 - ans) / 2.0;
			}
			// IVS failed and DVS succeeded
			if (ans < 0 && DVSpolar != 0)
			{
				Accumulator_b[idx] += (Lastgray[idx] + value) / 2.0;
				Accumulator_d[idx] += 255 - (Lastgray[idx] + value) / 2.0;
				for (const auto curr_t : DVS_times)
				{
					if (fabs(frames + curr_t - Reftime[idx]) < detect_wsize)
					{
						Eventrecord Dev(i, j, frames + curr_t, (DVSpolar + 1) / 2);
						HybridEvents.push_back(Dev);
						D_event++;
						DVSsave[idx] += DVSthres * DVSpolar;
						Reftime[idx] = frames + curr_t;
						double end_val = DVSsave[idx];
						Accumulator_b[idx] = (end_val + value) * (1 - curr_t) / 2.0;
						Accumulator_d[idx] = (510 - end_val - value) * (1 - curr_t) / 2.0;
					}
					else
					{
						DVSsave[idx] += DVSthres * DVSpolar;
					}
				}
			}
			// IVS succeeded and DVS succeeded
			if (ans >= 0 && DVSpolar != 0)
			{
				Eventrecord Iev(i, j, frames + ans, IVSpolar);
				HybridEvents.push_back(Iev);
				Reftime[idx] = frames + ans;
				I_event++;
				double end_val = Lastgray[idx] + ans * total_change;
				Accumulator_b[idx] = (end_val + value) * (1 - ans) / 2.0;
				Accumulator_d[idx] = (510 - end_val - value) * (1 - ans) / 2.0;
				for (const auto curr_t : DVS_times)
				{
					if (curr_t < ans)
					{
						// these event must be ignored
						// in case violate the machenism
						DVSsave[idx] += DVSthres * DVSpolar;
					}
					else if (fabs(frames + curr_t - Reftime[idx]) < detect_wsize)
					{
						Eventrecord Dev(i, j, frames + curr_t, (DVSpolar + 1) / 2);
						HybridEvents.push_back(Dev);
						D_event++;
						DVSsave[idx] += DVSthres * DVSpolar;
						Reftime[idx] = frames + curr_t;
						end_val = DVSsave[idx];
						Accumulator_b[idx] = (end_val + value) * (1 - curr_t) / 2.0;
						Accumulator_d[idx] = (510 - end_val - value) * (1 - curr_t) / 2.0;
					}
					else
					{
						DVSsave[idx] += DVSthres * DVSpolar;
					}
				}
			}
			Lastgray[idx] = value;
		}
	}
	frames++;
	
	for (const auto hev : HybridEvents)
	{
		if (!_finite(hev.t))
		{
			int idx = hev.x * height + hev.y;
			Accumulator_b[idx] = Accumulator_d[idx] = 0;
			DVSsave[idx] = Clamp(img.at<uchar>(hev.y, hev.x), 0, 255);
			Reftime[idx] = frames;
		}
		else
		{
			outstream << hev.x << " " << hev.y << " " << hev.t << " " << hev.p << ", ";
		}
	}

}
