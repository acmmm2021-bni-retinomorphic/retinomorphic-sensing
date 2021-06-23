#include "Simulatorbase.h"

void VidarSimulator::resetMemory(int width, int height, int channels)
{
	/*
	 * reset the accumulators to fit a new size of image
	 * caution: all value saved will be erased 
	 */
	if(channels != 1)
	{
		std::cout << "Error to reset with channels" << channels << ::std::endl;
		return;
	}
	if(width <= 0 || height <= 0)
	{
		std::cout << "Bad width or height" << width << " " << height << ::std::endl;
		return;
	}
	accumulator.resize(width * height, 0);
}

int VidarSimulator::SimulateEventFromImage(cv::Mat& img, ::std::ofstream& outstream)
{
	if(channels == 1)
	{
		SimulateoneChannel(img, outstream);
		return 0;
	}
	else
	{
		std::cout << "Error to simulate image with channels " << channels << ::std::endl
					<< "Please reset the simulator." << ::std::endl;
		return 1;
	}
}

void VidarSimulator::SimulateoneChannel(cv::Mat& img, ::std::ofstream& outstream)
{
	bit_buffer grayBits;
	//insert frames via linear method
	if (frames != 0)
	{
		for (int epoch = 0; epoch < linear_frames; epoch++)
		{
			for (int j = 0; j < height; j++)
			{
				for (int i = 0; i < width; i+=8)
				{
					for (int e = 7; e >= 0; e--)
					{
						if(i + e >= width)
							continue;				
						int idx = (i + e) * height + j;
						uint8_t _gray = img.at<uchar>(j, i + e);
						_gray = Clamp(_gray, 0, 255);
						double lin_val = ((epoch + 1) * Lastgray[idx] + (linear_frames - epoch) * (int)_gray) / (double)(linear_frames + 1);							
						accumulator[idx] += lin_val;
						if (accumulator[idx] > IVSthres)
						{
							grayBits.write_bits(true, 1);
							if (keepResidual)
								accumulator[idx] -= IVSthres;
							else
								accumulator[idx] = 0;
						}
						else
							grayBits.write_bits(false, 1);
					}
				}
			}
		}
	}
	frames++;

	// output bitstream
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i+=8)
		{
			for (int e = 7; e >= 0; e--)
			{
				if(i + e >= width)
					continue;				
				int idx = (i + e) * height + j;		
				uint8_t _gray = img.at<uchar>(j, i + e);
				_gray = Clamp(_gray, 0, 255);
				accumulator[idx] += _gray;
				Lastgray[idx] = _gray;
				if (accumulator[idx] > IVSthres)
				{
					grayBits.write_bits(true, 1);
					if (keepResidual)
						accumulator[idx] -= IVSthres;
					else
						accumulator[idx] = 0;
				}
				else
					grayBits.write_bits(false, 1);
			}
		}
	}

	for (auto& byte : grayBits.get_bytes())
	{
		outstream << (uint8_t)byte;
	}
}


bit_buffer::bit_buffer(const size_t size) :	pos_(0), bit_index_(0)
{
	this->buffer_.reserve(size);
}

bit_buffer::~bit_buffer() {}
