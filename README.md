There are two folders named Simulator and Reconmethod.
Both of them contain the C++ implementations of ATIS, DAVIS, Vidar and our retinomorphic sensing.
Specifically, the implement of our encoding framework is written by Python3 as RetinaSimulator.py and a C++ approximation is written in RetinaSimulator.cpp.
The simulators takes a series of images as input and outputs event or spike streams.
And the reconstruction method takes the output file as input and generates reconstruction images.
To run these simulators, OpenCV is required to read and write on images.

To run the simulators, Visual Studio for C++ is recommended.
First, create a new C++ blank project named "AERmethods" , for example.
Then add all .h files to the header files and all .c/.cpp files to the source files.
After that, you may need to add OpenCV to the project:
	Properties -> VC++ direcotries -> Include -> add the include directory of OpenCV
	Properties -> VC++ direcotries -> Library -> add the library directory of OpenCV
	Properties -> Linker -> Input -> add libraty name of OpenCV to "Additional Dependencies"
Finally, set Properties -> C/C++ -> Language -> Pemissive to false.
And you can build you project after finishing mentioned above.

To run RetinaSimulator.py, Python3 and OpenCV is required.

More details about the input parameters (such as image information and thresholds) of the simulators can refer to the Simulator\run.cpp, and for the reconstruction methods, please refer to the Reconmethod\Reconmain.cpp.