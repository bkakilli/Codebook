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
Linux
-----
Get into project directory ($CodebookPath)
> ./bin/Debug/codebook <video file> <Mode: p for playing, t for training>

Windows
-------
Add OpenCV dll's to path in environment variables
Enter command line and go to project directory where Codebook.cbp file exists
> bin/Debug/codebook <video file> <Mode: p for playing, t for training>
