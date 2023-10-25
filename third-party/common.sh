set -e

if [ -f ../../.env ]; then
    source ../../.env
elif [ -f ../../../.env ]; then
	source ../../../.env
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

export CONFIG_UCLIBC_BUILD=y

APP_DIR=$ZRT_ENV_TOP_DIR/70mai/
APP_INCLUDE_DIR=$APP_DIR/include

if [ "$CONFIG_UCLIBC_BUILD" = "n" ] ; then
INSTALL_DIR=$ZRT_ENV_TOP_DIR/70mai/third-party/target/glibc
APP_LIB_DIR=$APP_DIR/lib/glibc
else
INSTALL_DIR=$ZRT_ENV_TOP_DIR/70mai/third-party/target/uclibc
APP_LIB_DIR=$APP_DIR/lib/uclibc
fi

export CROSS_PREFIX=mips-linux-gnu-
export HOST=${CROSS_PREFIX%-}

[ ! -n "$ROOTFS" ] \
	&& ROOTFS=$ZRT_ENV_OS_DIR/rootfs/7.2.0/camera/rootfs_camera

[ ! -n "$USERFS" ] \
	&& USERFS=$ZRT_ENV_OS_DIR/rootfs/7.2.0/camera/system

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

setup()
{
	SRC_URL=$1
	SRC_TGZ=$(basename $SRC_URL)
	SRC_DIR=${SRC_TGZ%.tar.*}
	if [ ! -e $SRC_DIR ]; then
		[ ! -e $SRC_TGZ ] && wget $SRC_URL
		tar xf $SRC_TGZ
	fi
}
