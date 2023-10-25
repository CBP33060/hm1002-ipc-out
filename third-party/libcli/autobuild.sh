#!/bin/bash -e

source ../common.sh

build_libcli() {
	echo "[libcli] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/libcli &> install.log
	rm libCLILib.so
	echo "[libcli] done"
}

clean_libcli()
{
	make clean &>make.log
}

rootfs_install_libcli()
{
	echo "[libcli] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
	cp -rf $INSTALL_DIR/libcli/lib/libCLILib.so $APP_LIB_DIR
	cp -rf $INSTALL_DIR/libcli/include/* $APP_INCLUDE_DIR
}

install_libcli()
{
	rootfs_install_libcli
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libcli
elif type -t $1_libcli 2> /dev/null >&2 ; then
        $1_libcli
fi
