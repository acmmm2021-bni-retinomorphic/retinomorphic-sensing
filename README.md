There are two folders named Simulator and Reconmethod.
Both of them contain the c++ implementations of ATIS, DAVIS, Vidar and our retinomorphic sensing.
Specifically, the implement of our encoding framework is written by python as RetinaSimulator.py.
The simulators takes a series of images as input and outputs event or spike streams.
And the reconstruction method takes the output file as input and generates reconstruction images.
To run these simulators, OpenCV is required to read and write on images.

More details about the input parameters (such as image information and thresholds) of the simulators can refer to the Simulator\run.cpp, and for the reconstruction methods, please refer to the Reconmethod\Reconmain.cpp.