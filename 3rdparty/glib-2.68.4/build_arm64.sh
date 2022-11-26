#!/bin/sh


meson setup --prefix=$(pwd)/install_arm64 --cross-file cross_file_arm64.txt build_arm64
ninja -C build_arm64 && ninja install -C build_arm64

