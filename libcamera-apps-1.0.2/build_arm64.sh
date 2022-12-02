#!/bin/sh

mkdir build_arm64
cd build_arm64

cmake .. --install-prefix $(pwd)/../install_arm64 -DENABLE_DRM=1 -DENABLE_OPENCV=0 -DENABLE_TFLITE=0 -DENABLE_LIBAV=0 -DENABLE_X11=0 -DENABLE_QT=0 \
	&& make && make install

cd ../

