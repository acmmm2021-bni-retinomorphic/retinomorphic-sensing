#include "Reconbase.h"

void ReconATIS::GenerateImages(int totalframes, std::ifstream& Eventfile)
{
	double eps = 1e-6;
	cv::Mat** out_img;
	int opti_size = max_inte_time + 2;
	out_img = new Mat* [opti_size];
	for (int i = 0; i < opti_size; i++)
	{
		out_img[i] = new Mat(Size(width, height), CV_8UC1);
		for (int xx = 0; xx < width; xx++)
			for (int yy = 0; yy < height; yy++)
				(*out_img[i % opti_size]).at<uchar>(yy, xx) = 0;
	}
	
	bool* DVSstart;
	double* Starttime;
	DVSstart = new bool [width * height];
	Starttime = new double [width * height];
	memset(DVSstart, 0, sizeof(bool) * width * height);

	int x, y, p;
	double t;
	char comma = ',';
	int curmin = 0;
	char buffer[256];
	int cnt = 0;
	while(1)
	{
		cnt++;
		// Eventfile >> x >> y >> t >> p >> comma;
		Eventfile.getline(buffer, 256, ',');
		int succ = sscanf_s(buffer, "%d %d %lf %d", &x, &y, &t, &p);
		if (succ != 4)
		{
			std::cout << succ << std::endl;
			std::cout << buffer << std::endl;
			break;
		}
		int idx = x * height + y;
		if (Eventfile.eof() || t >= totalframes)
		{
			std::cout << x << " " << y << " " << t << " " << totalframes << std::endl;
			break;
		}
		if (t >= curmin + opti_size + eps)
		{
			int i;
			for(i = curmin; i + max_inte_time < t; i++)
			{
				sprintf_s(buffer, 256, output_fmt.c_str(), i);
				cv::imwrite(buffer, *out_img[i % opti_size]);

				for (int xx = 0; xx < width; xx++)
					for (int yy = 0; yy < height; yy++)
						(*out_img[i % opti_size]).at<uchar>(yy, xx) = 0;
			}
			curmin = i;
		}
		if (p == 0)
		{
			DVSstart[idx] = true;
			Starttime[idx] = t;
		} 
		else 
		{	
			if (DVSstart[idx] && t - Starttime[idx] <= max_inte_time)
			{
				int grayval = IVSthres / (t - Starttime[idx]);
				for (int i = (int)ceil(Starttime[idx]); i < t; i++)
				{
					(*out_img[i % opti_size]).at<uchar>(y, x) = grayval;
				}
			}
			DVSstart[idx] = false;
		}
	}

	
	for (int i = curmin; i < min(curmin + opti_size, totalframes); i++)
	{
		sprintf_s(buffer, 256, output_fmt.c_str(), i);
		cv::imwrite(buffer, *out_img[i % opti_size]);
	}
	
}