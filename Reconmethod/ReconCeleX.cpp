#include "Reconbase.h"

void ReconCeleX::GenerateImages(int totalframes, std::ifstream& Eventfile)
{
	cv::Mat out_img(Size(width, height), CV_8UC1);
	if (first_frame.length() == 0)
	{
		for (int xx = 0; xx < width; xx++)
			for (int yy = 0; yy < height; yy++)
				out_img.at<uchar>(yy, xx) = 0;
	}
	else
	{
		out_img = imread(first_frame, 0);
	}
	if (out_img.empty())
	{
		std::cout << "Failed to load file: " << first_frame << std::endl;
		return;
	}
	
	int x, y, p, val;
	double t;
	char comma = ',';
	int curmin = 0;
	char buffer[256];
	while(1)
	{
		Eventfile.getline(buffer, 256, ',');
		int succ = sscanf_s(buffer, "%d %d %lf %d %d", &x, &y, &t, &p, &val);
		if (succ != 5)
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
				sprintf_s(buffer, 256, output_fmt.c_str(), i);
				cv::imwrite(buffer, out_img);
			}
			curmin = t + 1;
		}

		out_img.at<uchar>(y, x) = clamp(val);
	}
	for (int i = curmin; i < totalframes; i++)
	{
		sprintf_s(buffer, 256, output_fmt.c_str(), i);
		cv::imwrite(buffer, out_img);
	}
}