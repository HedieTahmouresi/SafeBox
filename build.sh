#!/bin/bash
set -e  

echo "[BUILD] Cleaning build directory..."
rm -rf build
mkdir build
cd build

echo "[BUILD] Configuring with CMake..."
cmake ..

echo "[BUILD] Compiling..."
make

echo "[BUILD] Running SafeBox..."
#sudo ./safebox --cpu 20 --cmd /bin/sh
sudo ./safebox --cpu 20 --cmd /bin/sh
#sudo ./safebox --cmd /breakout