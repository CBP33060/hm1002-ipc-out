#!/bin/bash -e

source ../common.sh

build_libmp4v2()
{
	echo "[libmp4v2] libmp4v2"

	tar xf mp4v2-2.0.0.tar.gz
	pushd mp4v2-2.0.0
	if [ "$CONFIG_UCLIBC_BUILD" = "y" ] ; then
	CFLAGS=-muclibc 
	CXXFLAGS=-muclibc 
	else
	CFLAGS=
	CXXFLAGS=
	fi
	chmod 755 ./configure
	./configure CC=${CROSS_PREFIX}gcc CXX=${CROSS_PREFIX}g++ --host=$HOST CFLAGS=$CFLAGS CXXFLAGS=$CXXFLAGS &> configure.log
	
	echo "[libmp4v2] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/libmp4v2 &> install.log

	echo "[libmp4v2] done"
	popd
}

clean_libmp4v2()
{
	rm -rf mp4v2-2.0.0
}

rootfs_install_libmp4v2()
{
	echo "[libmp4v2] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
	cp -rf $INSTALL_DIR/libmp4v2/usr/local/lib/libmp4v2.so* $APP_LIB_DIR
	cp -rf $INSTALL_DIR/libmp4v2/usr/local/include/* $APP_INCLUDE_DIR
}

install_libmp4v2()
{
	rootfs_install_libmp4v2
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libmp4v2
elif type -t $1_libmp4v2 2> /dev/null >&2 ; then
	$1_libmp4v2
fi
