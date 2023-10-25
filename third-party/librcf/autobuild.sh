#!/bin/bash -e

source ../common.sh

build_librcf() {
	echo "[librcf] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/librcf &> install.log
	echo "[librcf] done"
}

clean_librcf()
{
	rm librcf.so *log > /dev/null 2>&1
	make clean > /dev/null 2>&1
}

rootfs_install_librcf()
{
	echo "[librcf] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
	cp -rf $INSTALL_DIR/librcf/lib/librcf.so $APP_LIB_DIR
	cp -rf ./include/* $APP_INCLUDE_DIR
}

install_librcf()
{
	rootfs_install_librcf
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_librcf
elif type -t $1_librcf 2> /dev/null >&2 ; then
        $1_librcf
fi
