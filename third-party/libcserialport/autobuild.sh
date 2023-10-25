#!/bin/bash -e

source ../common.sh

build_libcserialport() {
	echo "[libcserialport] make"
	make 
	make install-strip DESTDIR=$INSTALL_DIR/libcserialport &> install.log
	echo "[libcserialport] done"
}

clean_libcserialport()
{
	rm libcserialport.so *log > /dev/null 2>&1
	make clean > /dev/null 2>&1
}

rootfs_install_libcserialport()
{
	echo "[libcserialport] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
	cp -rf $INSTALL_DIR/libcserialport/lib/* $APP_LIB_DIR
	cp -rf $INSTALL_DIR/libcserialport/include/* $APP_INCLUDE_DIR
}

install_libcserialport()
{
	rootfs_install_libcserialport
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libcserialport
elif type -t $1_libcserialport 2> /dev/null >&2 ; then
        $1_libcserialport
fi
