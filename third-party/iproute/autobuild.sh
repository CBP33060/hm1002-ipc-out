#!/bin/bash -e

source ../common.sh

build_iproute() {
	pushd iproute2
    
	if [ "$CONFIG_UCLIBC_BUILD" = "y" ] ; then
	CFLAGS="-static -muclibc "
	CPPFLAGS=-muclibc
	LDFLAGS=-muclibc 
	else
	CFLAGS=
	CPPFLAGS=
	LDFLAGS=
	fi

	echo "[iproute] make"
	./configure --prefix=$(pwd)/result --libbpf_force=off #&> configure.log
	make CC="mips-linux-gnu-gcc" AR="mips-linux-gnu-ar" #&> make.log
	make install DESTDIR=$INSTALL_DIR/iproute &> install.log

	echo "[iproute] done"
}

clean_iproute()
{
	pushd iproute2
	make clean &>make.log
}

rootfs_install_iproute()
{
	echo "[iproute] rootfs_install"
	mkdir -p $ROOTFS/bin
	cp -a $INSTALL_DIR/iproute/sbin/tc $ROOTFS/bin
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_iproute
elif type -t $1_iproute 2> /dev/null >&2 ; then
        $1_iproute
fi
