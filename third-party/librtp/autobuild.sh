#!/bin/bash -e

source ../common.sh

build_librtp() {
	echo "[librtp] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/librtp &> install.log
	echo "[librtp] done"
}

clean_librtp()
{
	rm librtp.so *log > /dev/null 2>&1
	make clean > /dev/null 2>&1
}

rootfs_install_librtp()
{
	echo "[librtp] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
	cp -rf $INSTALL_DIR/librtp/lib/* $APP_LIB_DIR
	cp -rf $INSTALL_DIR/librtp/include/* $APP_INCLUDE_DIR
}

install_librtp()
{
	rootfs_install_librtp
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_librtp
elif type -t $1_librtp 2> /dev/null >&2 ; then
        $1_librtp
fi
