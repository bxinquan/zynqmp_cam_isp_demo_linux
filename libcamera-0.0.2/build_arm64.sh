#!/bin/sh


meson build_arm64 --prefix=/usr
ninja -C build_arm64 && DESTDIR=$(pwd)/install_arm64 ninja install -C build_arm64
sudo cp -rdfv install_arm64/* $PKG_CONFIG_SYSROOT_DIR

