echo off
if defined VisualStudioVersion echo %VisualStudioVersion% ELSE call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" amd64

set URHO3D_DIR=D:/projects/voxel/Urho3D
set POLYVOX_DIR=D:/projects/voxel/polyvox
set LIBNOISE_DIR=D:/projects/voxel/libnoise

set cmake_call=cmake -H. -B_buildx64 -G "Visual Studio 14 2015 Win64"^
 -DUSE_OCTOMAP=OFF^
 -DVOXIGEN_OUTPUT_DIRECTORY_DEBUG="E:/projects/lumberyard_git/dev/Bin64vc140.Debug"^
 -DVOXIGEN_OUTPUT_DIRECTORY_RELEASE="E:/projects/lumberyard_git/dev/Bin64vc140"

echo %cmake_call%
call %cmake_call%