#!/bin/bash -e

source ../common.sh

build_iperf3() {
	rm -rf iperf-3.0.11
    tar xvf iperf-3.0.11.tar.gz
	pushd iperf-3.0.11
    patch -p1 < ../iperf3_20230627.patch
    if [ "$CONFIG_UCLIBC_BUILD" = "y" ] ; then
	CFLAGS="-static -muclibc "
	CPPFLAGS=-muclibc
	LDFLAGS=-muclibc 
	else
	CFLAGS=
	CPPFLAGS=
	LDFLAGS=
	fi

	echo "[iperf3] configure"
	./configure CFLAGS="$CFLAGS" --host=${CROSS_PREFIX%-} --prefix=/ &> configure.log
	echo "[iperf3] make"
	make &> make.log
	# make install-strip DESTDIR=$INSTALL_DIR/iperf3 &> install.log

	echo "[iperf3] done"
}

clean_iperf3()
{
	rm -rf iperf-3.0.11
}

rootfs_install_iperf3()
{
	echo "[iperf3] rootfs_install"
	mkdir -p $ROOTFS/bin
	cp -a $INSTALL_DIR/iperf3/bin/iperf $ROOTFS/bin
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_iperf3
elif type -t $1_iperf3 2> /dev/null >&2 ; then
        $1_iperf3
fi
