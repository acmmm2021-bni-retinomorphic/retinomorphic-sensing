#include "Reconbase.h"
#include<stack>
#include<random>

std::default_random_engine gen;
std::normal_distribution<double> dis(0.4, 0.13);

void ReconRetina::GenerateImages(int totalframes, std::ifstream& EventsFile)
{
	static const double eps = 1e-6;
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
	double* Lastfinaltime;
	int* LastIval;
	std::stack<Eventrecord>** eventstack;
	eventstack = new std::stack<Eventrecord>* [width * height];
	for (int i = 0; i < width * height; i++)
	{
		eventstack[i] = new std::stack<Eventrecord>();
	}

	Lasttime = new double [width * height];
	Lastfinaltime = new double [width * height];
	LastIval = new int [width * height];
	for (int i = 0; i < width * height; i++)
	{
		Lastfinaltime[i] = 0;
		LastIval[i] = -1;
		Lasttime[i] = 0;
	}

	int x, y, p;
	double t;
	char comma = ',';
	int curmin = 0;
	char buffer[256];
	int cnt = 0;
	double symbolt = 0;
	while(1)
	{
		cnt++;
		// EventsFile >> x >> y >> t >> p >> comma;
		EventsFile.getline(buffer, 256, ',');
		int succ = sscanf_s(buffer, "%d %d %lf %d", &x, &y, &t, &p);
		if (succ != 4)
		{
			std::cout << succ << std::endl;
			std::cout << buffer << std::endl;
			break;
		}
		
		if (EventsFile.eof() || t >= totalframes)
		{
			std::cout << x << " " << y << " " << t << " " << totalframes << std::endl;
			break;
		}

		if (x < 0 || y < 0 || x >= width || y >= height || !_finite(t))
		{
			continue;
		}
		int idx = x * height + y;

		if (t > symbolt)
		{
			std::cout << t << " get!" << std::endl;
			symbolt += 10;
		}

		t = t + dis(gen);
		if (t < Lasttime[idx])
		{
			t = Lasttime[idx] + eps;
		}

		// D-event ?
		if (fabs(t - Lasttime[idx]) < window_size)
		{
			Eventrecord devent(x, y, t, p);
			(*eventstack[idx]).push(devent);
			Lasttime[idx] = t;
		}
		// I-event ?             
		else
		{
			double sevent_t = t;
			int sevent_v;

			// D+I format
			if (!(*eventstack[idx]).empty())
			{
				// restore values between D and S 
				Eventrecord tmp = (*eventstack[idx]).top();
				int val;
				if (p == 0)
				{
					val = 255 - IVSthres / (t - tmp.t); 
				}
				else
				{
					val = IVSthres / (t - tmp.t);
				}
				val = clamp(val);
				sevent_v = val;
				for (int i = ceil(tmp.t) + eps; i <= t + eps; i++)
				{
					(*out_img[i]).at<uchar>(y, x) = val;
				}
				t = tmp.t;
				p = tmp.p;
				(*eventstack[idx]).pop();

				// restore values between D-events
				double changepersec;
				while(!(*eventstack[idx]).empty())
				{
					tmp = (*eventstack[idx]).top();
					changepersec = DVSthres / (t - tmp.t);
					for (int i = floor(t) + eps; i >= tmp.t - eps; i--)
					{
						if (p == 0)
						{
							(*out_img[i]).at<uchar>(y, x) = clamp(val + (t - i) * changepersec);
						}
						else
						{
							(*out_img[i]).at<uchar>(y, x) = clamp(val - (t - i) * changepersec);
						}
					}
					val = clamp(val + (-2 * p + 1) * DVSthres);
					t = tmp.t;
					p = tmp.p;
					(*eventstack[idx]).pop();
				}

				// restore the value from last I to the first D
				// first null event
				// cannot refer the value
				if (LastIval[idx] < 0)
				{
					changepersec = DVSthres / (t - Lastfinaltime[idx]);
					for (int i = ceil(Lastfinaltime[idx]) + eps; i <= floor(t) + eps; i++)
					{
						if (p == 0)
						{
							(*out_img[i]).at<uchar>(y, x) = clamp(val + (t - i) * changepersec);
						}
						else
						{
							(*out_img[i]).at<uchar>(y, x) = clamp(val - (t - i) * changepersec);
						}
					}
				}
				// last I-event, the p of D-event is not accurate
				else
				{
					if (fabs(t - Lastfinaltime[idx]) > eps)
					{
						changepersec = (val - LastIval[idx]) / (t - Lastfinaltime[idx]);
						for (int i = ceil(Lastfinaltime[idx]) + eps; i <= floor(t) + eps; i++)
						{
							//(*out_img[i]).at<uchar>(y, x) = clamp(val + (i - t) * changepersec);
							(*out_img[i]).at<uchar>(y, x) = LastIval[idx];
						}
					}
				}
			}

			// I I format or null I format
			else
			{
				int val;
				if (p == 0)
				{
					val = 255 - IVSthres / (t - Lastfinaltime[idx]); 
				}
				else
				{
					val = IVSthres / (t - Lastfinaltime[idx]);
				}
				val = clamp(val);
				sevent_v = val;
				for (int i = ceil(Lastfinaltime[idx]) + eps; i <= floor(t) + eps; i++)
					(*out_img[i]).at<uchar>(y, x) = val;
			}
			Lastfinaltime[idx] = sevent_t;
			LastIval[idx] = sevent_v;
			Lasttime[idx] = sevent_t; 
		}
	}
	for (int i = 0; i < totalframes; i++)
	{
		sprintf_s(buffer, 256, output_fmt.c_str(), i);
		cv::imwrite(buffer, *out_img[i]);
	}
}
