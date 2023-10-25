#!/bin/bash -e

source ../common.sh

build_libjson() {
	echo "[libjson] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/libjson &> install.log
	echo "[libjson] done"
}

clean_libjson()
{
	rm libjson.so *log > /dev/null 2>&1
	make clean > /dev/null 2>&1
}

rootfs_install_libjson()
{
	echo "[libjson] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
	cp -rf $INSTALL_DIR/libjson/lib/* $APP_LIB_DIR
	cp -rf $INSTALL_DIR/libjson/include/* $APP_INCLUDE_DIR
}

install_libjson()
{
	rootfs_install_libjson
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libjson
elif type -t $1_libjson 2> /dev/null >&2 ; then
        $1_libjson
fi
