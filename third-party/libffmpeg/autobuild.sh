#!/bin/bash -e

source ../common.sh

build_libffmpeg()
{
	echo "[libffmpeg] libffmpeg"

	tar vxf ffmpeg-4.3.1.tar.bz2
	pushd ffmpeg-4.3.1
	if [ "$CONFIG_UCLIBC_BUILD" = "y" ] ; then
	CFLAGS=-muclibc 
	CPPFLAGS=-muclibc
	LDFLAGS=-muclibc 
	else
	CFLAGS=
	CPPFLAGS=
	LDFLAGS=
	fi
	chmod 755 ./configure
	CFLAGS=$CFLAGS CPPFLAGS=$CPPFLAGS LDFLAGS=$LDFLAGS ./configure --enable-cross-compile  --arch=mips --target-os=linux \
	--cross-prefix=${CROSS_PREFIX} --enable-shared --disable-static --enable-gpl --disable-decoders \
	--enable-decoder=hevc --enable-decoder=opus --enable-decoder=aac --enable-decoder=h264 --disable-encoders \
	--disable-demuxers --enable-demuxer=ogg --enable-demuxer=hevc --enable-demuxer=aac --enable-demuxer=h264 \
	--disable-muxers --enable-muxer=mp4  --disable-mipsfpu --disable-x86asm --disable-programs --disable-mipsdsp \
	--disable-mipsdspr2 --disable-mips32r2 --disable-mips64r2 --disable-mips32r6

	echo "[libffmpeg] make"
	make -j6
	make install DESTDIR=$INSTALL_DIR/libffmpeg

	echo "[libffmpeg] done"
	popd
}

clean_libffmpeg()
{
	rm -rf ffmpeg-4.3.1
	make uninstall > /dev/null 2>&1
	make clean > /dev/null 2>&1
}

rootfs_install_libffmpeg()
{
	echo "[libffmpeg] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR/ffmpeg
	cp -f $INSTALL_DIR/libffmpeg/usr/local/lib/libavformat.so* -d $APP_LIB_DIR
	cp -f $INSTALL_DIR/libffmpeg/usr/local/lib/libavcodec.so* -d $APP_LIB_DIR
	cp -f $INSTALL_DIR/libffmpeg/usr/local/lib/libswresample.so* -d $APP_LIB_DIR
	cp -f $INSTALL_DIR/libffmpeg/usr/local/lib/libavutil.so* -d $APP_LIB_DIR
	cp -rf $INSTALL_DIR/libffmpeg/usr/local/include/* $APP_INCLUDE_DIR/ffmpeg
}

install_libffmpeg()
{
	rootfs_install_libffmpeg
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libffmpeg
elif type -t $1_libffmpeg 2> /dev/null >&2 ; then
	$1_libffmpeg
fi
