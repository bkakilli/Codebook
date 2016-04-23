# Codebook
C++ implementation of "Codebook algorithm" for background substraction proposed in the paper "Real-time foreground–background segmentation using codebook model"

Installation
============

Linux
-----
Build and install OpenCV 3.1
clone repo to $CodebookPath

> cd CodebookPath
>
> cmake .
>
> make

Binary is created in $CodebookPath/bin

Windows
-------
Requires CodeBlocks)
Build OpenCV 3.1 for CodeBlocks using MinGW and CMake
Edit library and include directories in Codebook.cbp file

Open CodeBlocks project (Codebook.cbp)
Build project

Binary is created in bin/Debug or bin/Release

Usage
=====
(OpenCV required!)
> $PathToDirectory>Codebook source mode arguments
	
source: video file or folder containing images (required)<br />
mode: (t/p) t is for training p is for background substraction (required)<br />
arguments:<br />
* -f filename (required)
* -s training-test set separation percentage (default: 50)
* -a alpha value (default: 0.5)
* -b beta value (default: 1.4)
* -e epsilon value (default: 20)
* -m morphology operation
  * 1: Dilation
  * 2: Erosion
  * 3: Opening
  * 4: Closing
  * Default: No morphology

Examples
--------
Background detection without morphology with separation 10% (Frames after 10% of the video are used)
> PathToDirectory>Codebook.exe video p -f trained.cbf -s 10

Background detection with closing with separation 20% (Frames after 20% of the video are used)
> PathToDirectory>Codebook.exe video p -f trained.cbf -s 20 -m 4

Training with separation 25% (Frames before 25% of the video are used for training)
> PathToDirectory>Codebook.exe video t -f newTrainingFile.cbf -a 0.7 -b 1.1 -e 10 -s 25
