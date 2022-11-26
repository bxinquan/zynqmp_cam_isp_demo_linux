#!/bin/sh


export PATH=$PATH:$(pwd)/../glib-2.68.4/install_arm64/bin
meson setup --prefix=$(pwd)/install_arm64 --cross-file cross_file_arm64.txt build_arm64
ninja -C build_arm64 && ninja install -C build_arm64

