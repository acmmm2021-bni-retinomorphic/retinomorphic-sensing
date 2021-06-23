#include "Reconbase.h"

void ReconDAVIS::GenerateImages(int totalframes, std::ifstream& Eventfile)
{
	cv::Mat out_img;

	char buffer[256];
	int merge_idx = 0;
	sprintf_s(buffer, 256, input_fmt.c_str(), merge_idx);
	std::string input_file = buffer;
	out_img = imread(input_file, 0);
	
	int x, y, p, val;
	double t;
	char comma = ',';
	int curmin = 0;
	
	while(1)
	{
		Eventfile.getline(buffer, 256, ',');
		int succ = sscanf_s(buffer, "%d %d %lf %d", &x, &y, &t, &p);
		if (succ != 4)
		{
			std::cout << succ << std::endl;
			std::cout << buffer << std::endl;
			break;
		}
		if (Eventfile.eof() || t >= totalframes)
		{
			std::cout << x << " " << y << " " << t << " " << totalframes << std::endl;
			break;
		}
		if (x < 0 || y < 0 || x >= width || y >= height || !_finite(t))
		{
			continue;
		}

		if (t >= curmin)
		{
			for (int i = curmin; i <= t; i++)
			{
				if (i >= (merge_idx + 1) * merge_image_frames)
				{
					merge_idx++;
					sprintf_s(buffer, 256, input_fmt.c_str(), merge_idx);
					input_file = buffer;
					out_img = cv::imread(input_file, 0);
				}
				sprintf_s(buffer, 256, output_fmt.c_str(), i);
				cv::imwrite(buffer, out_img);
			}
			curmin = t + 1;
		}
		if (p == 0 || p == -1)
		{
			out_img.at<uchar>(y, x) = clamp((out_img).at<uchar>(y, x) - DVSthres);
		}
		else
		{
			out_img.at<uchar>(y, x) = clamp((out_img).at<uchar>(y, x) + DVSthres);
		}
	}
	for (int i = curmin; i < totalframes; i++)
	{
		if (i >= (merge_idx + 1) * merge_image_frames)
		{
			merge_idx++;
			sprintf_s(buffer, 256, input_fmt.c_str(), merge_idx);
			input_file = buffer;
			out_img = cv::imread(input_file, 0);
		}
		sprintf_s(buffer, 256, output_fmt.c_str(), i);
		cv::imwrite(buffer, out_img);
	}
}