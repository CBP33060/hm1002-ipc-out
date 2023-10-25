#!/bin/bash -e

source ../common.sh

build_libimgalgo() {
	echo "[libimgalgo] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/libimgalgo &> install.log
	echo "[libimgalgo] done"
}

clean_libimgalgo()
{
	rm libimgalgo.so *log > /dev/null 2>&1
	make clean > /dev/null 2>&1
}

rootfs_install_libimgalgo()
{
	echo "[libimgalgo] rootfs_install"

	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR/imgalgo
	cp -rf $INSTALL_DIR/libimgalgo/lib/* $APP_LIB_DIR
	cp -rf ./InferenceKit/include/* $APP_INCLUDE_DIR/imgalgo
	cp -rf ./InferenceKit/lib/uclibc/libvenus.so $APP_LIB_DIR
	cp -rf ./InferenceKit/lib/uclibc/libaip.so $APP_LIB_DIR
	cp -rf ./InferenceKit/lib/uclibc/libdrivers.so $APP_LIB_DIR
	cp -rf ./InferenceKit/lib/uclibc/libmi_alg_t41_detector_sdk.so $APP_LIB_DIR
	cp -rf ./InferenceKit/lib/uclibc/libalgsdk.so $APP_LIB_DIR
	cp -rf stb $APP_INCLUDE_DIR/imgalgo
	cp -rf $INSTALL_DIR/libimgalgo/include/* $APP_INCLUDE_DIR/imgalgo
	rm -rf $APP_INCLUDE_DIR/imgalgo/Eigen
}

install_libimgalgo()
{
	rootfs_install_libimgalgo
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libimgalgo
elif type -t $1_libimgalgo 2> /dev/null >&2 ; then
        $1_libimgalgo
fi
