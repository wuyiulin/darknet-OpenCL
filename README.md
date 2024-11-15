# Windows 10 setting guide

## Step0 install OpenCL
### [Download OpenCL](https://github.com/GPUOpen-LibrariesAndSDKs/OCL-SDK/releases)
install it and add OpenCL include and lib in to system Path

## Step1 install MSVC 
### [Download MSVC](https://visualstudio.microsoft.com/zh-hant/visual-cpp-build-tools/)

## Step2 install CMake
### [Download CMake](https://cmake.org/download/)

## Step3 Download cable
put in .\darknet-OpenCL\cmake</br>
should be a fold like .\darknet-OpenCL\cmake\cable
### [Download cable](https://github.com/sowson/cable/tree/0372b82f8f6148850898affe717157ca4c1b1574)

## Step4 install Boost
### Using pre-compile Boost 
```
cd your_path
```

#### [Download](https://sourceforge.net/projects/boost/files/boost-binaries/) pre-build 1.64.0 version and extract in curr path.

### Or compile Boost(using MSVC, Boost_1_64_0 Optional)
```
cd your_path
```
#### [Download](https://www.boost.org/users/history/) 1.64.0 version and extract in curr path.

```
cd boost_1_64_0

.\bootstrap.bat

.\b2 --prefix=your_install_path\boost_1_64_0 install
```

## Step5 install OpenCV
### Using pre-compile Boost
```
cd your_path
```
#### [Download](https://sourceforge.net/projects/boost/files/boost-binaries/) pre-build 1.64.0 version and extract in curr path.
### Or compile OpenCV(using MSVC, OpenCV4.10.0 Optional)
```
cd your_path
```
#### [Download](https://opencv.org/releases/) source code.

```
cd opencv4.10.0

mkdir build

cd build

cmake .. -G "Visual Studio 17 2022" -A x64 -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=../install -D WITH_OPENCL=ON -D BUILD_opencv_videoio=ON -D ENABLE_CXX11=ON -D OPENCV_ENABLE_NONFREE=ON -D BUILD_SHARED_LIBS=ON -DWITH_STATIC_LIBS=ON ..

cmake --build . --config Release --target INSTALL --clean-first
```
### Compile Darknet
```
cd ~/darknet-master
```
#### Edit CMakeList.txt and update OpenCL, Boost, OpenCV paths of build.

```

cmake -S .. -G "Visual Studio 17 2022"  -A x64

cmake --build ./ --config Release --target darknet --clean-first

copy Release/darknet.exe ../darknet.exe

copy ../3rdparty/clBLAS/clBLAS.dll ..

copy ../3rdparty/pthreads/pthreads.dll ..

cd ..
```

Than try
```
./darknet.exe
```

## Note
If you are using Windows 10 with Visual Studio Build Tools 2022 (MSVC 17),</br> 
the compatible OpenCV version is currently limited to 4.10.0. </br>
Older versions may fail to compile and are incompatible with prebuilt binaries.

Similarly, Boost 1.64.0 fails to compile with MSVC 17; please use a prebuilt version instead.

If you have any questions about building, feel free to contact me at wuyiulin@gmail.com