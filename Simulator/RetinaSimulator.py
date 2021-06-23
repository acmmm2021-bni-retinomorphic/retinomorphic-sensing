import math
import torch
import cv2 as cv
from torch import nn
import numpy as np

class Pixel_RNNcontrol(nn.Module):
	def __init__(self, isize = 2, hsize = 2, layn = 1):
		super(Pixel_RNNcontrol, self).__init__()
		self.rnn = nn.RNN(
			input_size = isize,
			hidden_size = hsize,
			num_layers = layn,
			nonlinearity = 'tanh'
		)
		self.set_rnnarguments()

	def forward(self, x, h):
		# x, input, [] seq_length, batch_size, input_size
		# h, hidden, [] num_layers, batch_size, hidden_size
		# out, output, [] seq_length, batch_size, hidden_size
		rnnout, hidden = self.rnn(x, h)
		out = torch.sigmoid(10 * rnnout)
		return out, hidden

	def set_rnnarguments(self):
		mydict = self.rnn.state_dict()
		testdict = {k: v for k, v in mydict.items()}
		for keys in testdict.keys():
			#print(keys, testdict[keys].shape)
			if keys[0] == 'w':
				testdict[keys] = torch.zeros_like(testdict[keys])
				for i in range(testdict[keys].shape[0]):
					testdict[keys][i][i] = 1
				if keys[7] == 'i':
					testdict[keys][0][0] = -1
			else:
				testdict[keys] = torch.zeros_like(testdict[keys])
		mydict.update(testdict)
		self.rnn.load_state_dict(mydict)

def special_float(x):
	return math.isnan(x) or math.isinf(x)

def find_result(x1, x2, y1, y2, ans):
	eps = 1e-6
	if ans < 0:
		return 0
	delta_x = x2 - x1
	max_inte = delta_x * (y1 + y2) / 2
	# no solution
	if max_inte + eps < ans:
		return -1
	# y = b
	if abs(y1 - y2) < eps and abs(y1) > eps:
		return ans / y1 + x1
	# y = kx + b
	k = (y2 - y1) / delta_x
	b = y1 - k * x1
	# kt^2 + 2bt - (kx1^2 + 2bx1 + 2 * ans) = 0
	c = (k * x1 + 2 * b) * x1 + 2 * ans
	if b*b + k*c < eps or abs(k) < eps:
		return -1
	res1 = (-b + math.sqrt(b*b + k*c)) / k
	res2 = (-b - math.sqrt(b*b + k*c)) / k
	if (not special_float(res1)) and res1 >= x1 and res1 <= x2:
		return res1
	elif (not special_float(res2)) and res2 >= x1 and res2 <= x2:
		return res2
	else:
		return -1

def clamp(x):
	if x > 255:
		return 255
	elif x < 0:
		return 0
	else:
		return x

class Pixel_Simulator:
	def __init__(self, IVSthres = 1020.0, DVSthres = 10, Windowsize = 3.8, Control = None):
		self.IVSthres = IVSthres
		self.DVSthres = DVSthres
		self.Windowsize = Windowsize
		self.Reftime = 0.0
		self.Lastgray = 0
		self.DVSsave = 0
		self.frames = 0
		self.Accumulator_b = 0.0
		self.Accumulator_d = 0.0
		self.Control = Control

	def Process_Pixel(self, value):
		recordlist = []
		value = int(value)
		if self.frames == 0:
			self.Lastgray = value
			self.DVSsave = value
			self.frames += 1
			self.Reftime = 0.0
			self.Accumulator_b = 0.0
			self.Accumulator_d = 0.0
			return recordlist

		# IVS part
		ans1 = find_result(0, 1, self.Lastgray, value, self.IVSthres - self.Accumulator_b)
		ans2 = find_result(0, 1, 255 - self.Lastgray, 255 - value, self.IVSthres - self.Accumulator_d)
		ans = ans1
		IVSpolar = 1
		if ans < 0 or (ans2 < ans and ans2 >= 0) :
			ans = ans2
			IVSpolar = 0

		# DVS part
		DVSpolar = 0
		DVS_times = []
		total_change = value - self.Lastgray
		if value - self.DVSthres >= self.DVSsave or value + self.DVSthres <= self.DVSsave:
			DVSpolar = 1 if value > self.DVSsave else -1
			if total_change == 0:
				strange_events = (self.Lastgray - self.DVSsave) * DVSpolar / self.DVSthres
				for i in range(int(strange_events)):
					DVS_times.append(0)
			else:
				current_event = self.DVSsave + self.DVSthres * DVSpolar
				start_time = (current_event - self.Lastgray) / float(total_change)
				while start_time >= 0 and start_time <= 1:
					DVS_times.append(start_time)
					start_time += self.DVSthres * DVSpolar / float(total_change)
		
		eps = 1e-6

		# IVS failed and DVS failed
		if ans < 0 and DVSpolar == 0:
			# integrating
			self.Accumulator_b += (self.Lastgray + value) / 2.0
			self.Accumulator_d += 255 - (self.Lastgray + value) / 2.0

		# IVS succeeded and DVS failed
		if ans >= 0 and DVSpolar == 0:
			status = torch.from_numpy(np.array([[[ans - self.Windowsize, 1.0]]]))
			status = torch.tensor(status, dtype = torch.float32)
			hidden = torch.from_numpy(np.array([[[self.Reftime, 0.0]]]))
			hidden = torch.tensor(hidden, dtype = torch.float32)
			out, hidden = self.Control.forward(status, hidden)

			symbol = out.detach().numpy().max()
			if symbol > 0.5 + eps:
				recordlist.append([self.frames + ans, IVSpolar])
			self.Reftime = symbol * (self.frames + ans) + (1 - symbol) * self.Reftime
			end_val = self.Lastgray + ans * total_change
			self.Accumulator_b = (1 - symbol) * self.Accumulator_b + symbol * (end_val + value) * (1 - ans) / 2.0
			self.Accumulator_d = (1 - symbol) * self.Accumulator_d + symbol * (510 - end_val - value) * (1 - ans) / 2.0

		# IVS failed and DVS succeeded
		if ans < 0 and DVSpolar != 0:
			self.Accumulator_b += (self.Lastgray + value) / 2.0
			self.Accumulator_d += 255 - (self.Lastgray + value) / 2.0
			for curr_t in DVS_times:
				status = torch.from_numpy(np.array([[[curr_t - self.Windowsize, -1.0]]]))
				status = torch.tensor(status, dtype = torch.float32)
				hidden = torch.from_numpy(np.array([[[self.Reftime, 0.0]]]))
				hidden = torch.tensor(hidden, dtype = torch.float32)
				out, hidden = self.Control.forward(status, hidden)
				symbol = out.detach().numpy().max()
				if symbol > 0.5 + eps:
					recordlist.append([self.frames + curr_t, (DVSpolar + 1) / 2])
				self.Reftime = symbol * (self.frames + curr_t) + (1 - symbol) * self.Reftime
				self.DVSsave += self.DVSthres * DVSpolar
				end_val = self.DVSsave
				# reset when symbol == 1, skip when symbol == 0
				self.Accumulator_b = (1 - symbol) * self.Accumulator_b + symbol * (end_val + value) * (1 - curr_t) / 2.0
				self.Accumulator_d = (1 - symbol) * self.Accumulator_d + symbol * (510 - end_val - value) * (1 - curr_t) / 2.0

		# IVS succeeded and DVS succeeded
		if ans >= 0 and DVSpolar != 0:
			status = torch.from_numpy(np.array([[[ans - self.Windowsize, 1.0]]]))
			status = torch.tensor(status, dtype = torch.float32)
			hidden = torch.from_numpy(np.array([[[self.Reftime, 0.0]]]))
			hidden = torch.tensor(hidden, dtype = torch.float32)
			out, hidden = self.Control.forward(status, hidden)

			symbol = out.detach().numpy().max()
			if symbol > 0.5 + eps:
				recordlist.append([self.frames + ans, IVSpolar])
			self.Reftime = symbol * (self.frames + ans) + (1 - symbol) * self.Reftime
			end_val = self.Lastgray + ans * total_change
			self.Accumulator_b = (1 - symbol) * self.Accumulator_b + symbol * (end_val + value) * (1 - ans) / 2.0
			self.Accumulator_d = (1 - symbol) * self.Accumulator_d + symbol * (510 - end_val - value) * (1 - ans) / 2.0

			for curr_t in DVS_times:
				if curr_t < ans:
					# these event must be ignored
					# in case violate the machenism
					self.DVSsave += self.DVSthres * DVSpolar
				else:
					status = torch.from_numpy(np.array([[[curr_t - self.Windowsize, -1.0]]]))
					status = torch.tensor(status, dtype = torch.float32)
					hidden = torch.from_numpy(np.array([[[self.Reftime, 0.0]]]))
					hidden = torch.tensor(hidden, dtype = torch.float32)
					out, hidden = self.Control.forward(status, hidden)
					symbol = out.detach().numpy().max()
					if symbol > 0.5 + eps:
						recordlist.append([self.frames + curr_t, (DVSpolar + 1) / 2])
					self.Reftime = symbol * (self.frames + curr_t) + (1 - symbol) * self.Reftime
					self.DVSsave += self.DVSthres * DVSpolar
					end_val = self.DVSsave
					# reset when symbol == 1, skip when symbol == 0
					self.Accumulator_b = (1 - symbol) * self.Accumulator_b + symbol * (end_val + value) * (1 - curr_t) / 2.0
					self.Accumulator_d = (1 - symbol) * self.Accumulator_d + symbol * (510 - end_val - value) * (1 - curr_t) / 2.0
		
		self.Lastgray = value
		self.frames += 1
		return recordlist

class Sensor_Simulator:
	def __init__(self, Width = 1020, Height = 720, IVSthres = 1020.0, DVSthres = 10, Windowsize = 3.8, Name = "default"):
		self.Width = Width
		self.Height = Height
		self.IVSthres = IVSthres
		self.DVSthres = DVSthres
		self.Windowsize = Windowsize
		self.Name = Name
		self.Control = None
		if self.Name == "default":
			self.Control = Pixel_RNNcontrol(isize = 2, hsize = 2, layn = 1)
		self.Pixel_List = [[Pixel_Simulator(IVSthres = self.IVSthres, DVSthres = self.DVSthres, Windowsize = self.Windowsize, Control = self.Control) for j in range (self.Width)] for i in range(self.Height)]

	def Process_Image(self, Imgname, Outfilename):
		img = cv.imread(Imgname, 0)
		if img is None:
			print("Bad name of img")
			return -1
		fp = open(Outfilename, "a")
		if fp is None:
			print("Bad name of file")
			return -1
		height = img.shape[0]
		width = img.shape[1]
		if height != self.Height and width != self.Width:
			img = cv.resize(img, (self.Width, self.Height))
		for i in range(self.Height):
			for j in range(self.Width):
				value = img[i][j]
				eventlist = self.Pixel_List[i][j].Process_Pixel(value)
				for event in eventlist:
					fp.write(str(i) + " " + str(j) + " " + str(event[0]) + " " + str(event[1]) + ", ")
		fp.close()

if __name__ == '__main__':
	filefmt = "E:\\E\\AERdata\\high-speed_videos\\bullet\\bullet%06d.png"
	output = "test.dat"
	end = 10
	Simulator = Sensor_Simulator(Width = 640, Height = 206, IVSthres = 1020.0, DVSthres = 10, Windowsize = 3.8, Name = "default")
	for i in range(end):
		filename = ("E:\\E\\AERdata\\high-speed_videos\\bullet\\bullet%06d.png") % (i + 1)
		Simulator.Process_Image(filename, output)
		print(i)
