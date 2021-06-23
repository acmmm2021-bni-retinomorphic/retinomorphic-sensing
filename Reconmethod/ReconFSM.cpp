#include "Reconbase.h"

void ReconFSM::GenerateImages(int totalframes, std::ifstream& Eventfile)
{
	double eps = 1e-6;
	cv::Mat** out_img;
	out_img = new Mat* [totalframes];

	for (int i = 0; i < totalframes; i++)
	{
		out_img[i] = new Mat(Size(width, height), CV_8UC1);
		for (int xx = 0; xx < width; xx++)
			for (int yy = 0; yy < height; yy++)
				(*out_img[i]).at<uchar>(yy, xx) = 0;
	}
	
	double* Lasttime;
	Lasttime = new double [width * height];
	for (int i = 0; i < width * height; i++)
	{
		Lasttime[i] = 0;
	}

	int x, y, p;
	double t;
	char comma = ',';
	int curmin = 0;
	char buffer[256];
	while(1)
	{
		// Eventfile >> x >> y >> t >> p >> comma;
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

		int idx = x * height + y;
		double delta_t = t - Lasttime[idx];
		double val = IVSthres / (delta_t + eps);
		for (int i = ceil(Lasttime[idx]); i <= t; i++)
		{
			(*out_img[i]).at<uchar>(y, x) = clamp((int)val);
		}
		Lasttime[idx] = t;
	}
	for (int i = 0; i < totalframes; i++)
	{
		sprintf_s(buffer, 256, output_fmt.c_str(), i);
		cv::imwrite(buffer, *out_img[i]);
	}
}