#!/bin/sh


./configure --host=aarch64-linux-gnu --prefix=/home/bxinquan/zynqmp_cam_isp_demo_linux/3rdparty/libevent-2.1.12-stable/install_arm64 --disable-openssl
make && make install

