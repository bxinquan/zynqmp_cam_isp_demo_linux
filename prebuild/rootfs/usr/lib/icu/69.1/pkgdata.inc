GENCCODE_ASSEMBLY_TYPE=-a gcc
SO=so
SOBJ=so
A=a
LIBPREFIX=lib
LIB_EXT_ORDER=.69.1
COMPILE=aarch64-xilinx-linux-gcc  -mcpu=cortex-a72.cortex-a53 -march=armv8-a+crc -fstack-protector-strong  -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -Wdate-time  -D_REENTRANT  -DU_HAVE_ELF_H=1 -DU_HAVE_STRTOD_L=1 -DU_HAVE_XLOCALE_H=0  -DU_ATTRIBUTE_DEPRECATED= -O2 -pipe -g -feliminate-unused-debug-types  -std=c11 -Wall -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings   -c
LIBFLAGS=-I/usr/include -DPIC -fPIC
GENLIB=aarch64-xilinx-linux-gcc  -mcpu=cortex-a72.cortex-a53 -march=armv8-a+crc -fstack-protector-strong  -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -Wdate-time  -O2 -pipe -g -feliminate-unused-debug-types  -std=c11 -Wall -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings   -Wl,-O1 -Wl,--hash-style=gnu   -Wl,-z,relro,-z,now  -shared -Wl,-Bsymbolic
LDICUDTFLAGS=
LD_SONAME=-Wl,-soname -Wl,
RPATH_FLAGS=
BIR_LDFLAGS=-Wl,-Bsymbolic
AR=aarch64-xilinx-linux-gcc-ar
ARFLAGS=r
RANLIB=aarch64-xilinx-linux-gcc-ranlib
INSTALL_CMD=install -c
