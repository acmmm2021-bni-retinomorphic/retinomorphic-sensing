# Code for retinomorphic sensing in MM BNI 2021
There are two folders named Simulator and Reconmethod.
Both of them contain the C++ implementations of [ATIS](https://ieeexplore.ieee.org/document/5648367), [DAVIS](https://ieeexplore.ieee.org/document/6889103), [Vidar](https://www.researchgate.net/publication/350834690_Spike_Camera_and_Its_Coding_Methods) and our retinomorphic sensing.
Our framework represents as follow:

# Framework
![Framework](Framework/framework.png?raw=true "framework")
Specifically, the implement of our encoding framework is written by Python3 in [RetinaSimulator.py](Simulator/RetinaSimulator.py) and a C++ approximation is written in [RetinaSimulator.cpp](Simulator/RetinaSimulator.cpp).

# Dataset and Results
We select several clips of videos, which are captured by a high-speed camera and processed into slow motion from [iX Cameras High-speed Video Gallery](https://www.ix-cameras.com/high_speed_camera_slow_motion_video_gallery.php).
Our framework can output asynchronous events for human vision (reconstruction) and machine vision (dynamic information).
![Visualization](Visualization/visualization_results.png?raw=true "framework")

# Requirement
To run the simulators, Visual Studio for C++ is recommended.
First, create a new C++ blank project named "AERmethods" , for example.
Then add all .h files to the header files and all .c/.cpp files to the source files.
After that, you may need to add OpenCV to the project:
- Properties -> VC++ direcotries -> Include -> add the include directory of OpenCV
- Properties -> VC++ direcotries -> Library -> add the library directory of OpenCV
- Properties -> Linker -> Input -> add libraty name of OpenCV to "Additional Dependencies"
In addition,
- Properties -> C/C++ -> Language -> Pemissive to false.
And you can build you project after finishing mentioned above.
To run these simulators, [OpenCV](https://opencv.org/) is required to read and write on images.
To run RetinaSimulator.py, Python3 with OpenCV is required.

# Running
The simulators takes a series of images as input and outputs event or spike streams.
And the reconstruction method takes the output file as input and generates reconstruction images.
More details about the input parameters (such as image information and thresholds) of the simulators can refer to the [run.cpp](Simulator\run.cpp), and for the reconstruction methods, please refer to the [Reconmain.cpp](Reconmethod\Reconmain.cpp).
