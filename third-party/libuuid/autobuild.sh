#!/bin/bash -e

source ../common.sh

build_libuuid()
{
	echo "[libuuid] libuuid"

	tar xf libuuid-1.0.3.tar.gz
	pushd libuuid-1.0.3
	if [ "$CONFIG_UCLIBC_BUILD" = "y" ] ; then
	CFLAGS=-muclibc
	else
	CFLAGS=
	fi
	./configure --enable-cross-compile --enable-shared --without-ssl --without-zlib --host=$HOST CFLAGS=$CFLAGS &> configure.log

	echo "[libuuid] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/libuuid &> install.log

	echo "[libuuid] done"
	popd
}

clean_libuuid()
{
	rm -rf libuuid-1.0.3
}

rootfs_install_libuuid()
{
	echo "[libuuid] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
	cp -rf $INSTALL_DIR/libuuid/usr/local/lib/libuuid.so* $APP_LIB_DIR
	cp -rf $INSTALL_DIR/libuuid/usr/local/include/* $APP_INCLUDE_DIR
}

install_libuuid()
{
	rootfs_install_libuuid
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libuuid
elif type -t $1_libuuid 2> /dev/null >&2 ; then
	$1_libuuid
fi
