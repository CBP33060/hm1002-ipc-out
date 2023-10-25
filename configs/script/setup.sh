#!/bin/bash
set -e

if [ -f .env ]; then
    source .env
    source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo -e "\033[31m"[ERROR]Please run source at top of zeratul project"\033[0m"
    exit 1
fi

if [ $# != 1 ]; then
    logger_info error_info "Please enter the compile type"
    exit 1
fi

APP_CONFIG_DIR=$ZRT_ENV_TOP_DIR/70mai/configs
INTERFACE_CONFIG_DIR=$ZRT_ENV_TOP_DIR/70mai/configs/media_interface
APP_BIN_DIR=$ZRT_ENV_TOP_DIR/70mai/bin/uclibc
LIB_DIR=$ZRT_ENV_TOP_DIR/70mai/lib/uclibc
LKM_DIR=$ZRT_ENV_TOP_DIR/70mai/lkm/modules/${ZRT_ENV_TOOL_CHAIN}/${ZRT_ENV_LINUX_VERSION//_/.}
ZERATUL_LIB_DIR=$ZRT_ENV_TOP_DIR/70mai/zeratul-sdk/lib/uclibc
TOOLS_BIN_DIR=$ZRT_ENV_TOP_DIR/70mai/tools

function install_app_to_rootfs()
{
    cp $APP_BIN_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/bin
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/bin/$1
}

function install_lib_to_rootfs()
{
    cp $LIB_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib/$1
}

function install_libdir_to_rootfs()
{
    cp -r $LIB_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib/$1/*
    #cp -r $INTERFACE_CONFIG_DIR/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib/$1/
}

function install_zeratul_lib_to_rootfs()
{
    cp $ZERATUL_LIB_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib/$1
}

function install_lkm_to_rootfs()
{
    cp $LKM_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib/modules
    mips-linux-gnu-strip --strip-debug $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/lib/modules/$1
}

function install_app_to_system()
{
    cp $APP_BIN_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/system/bin
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/system/bin/$1
}

function install_lib_to_system()
{
    cp $LIB_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/system/lib
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/system/lib/$1
}

function install_libffmpeg_to_system()
{
    cp -d $LIB_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/system/lib
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/system/lib/$1
}

function install_lkm_to_system()
{
    cp $LKM_DIR/$1 $ZRT_ENV_OUT_CAMERA_DIR/system/lib/modules
    mips-linux-gnu-strip --strip-debug $ZRT_ENV_OUT_CAMERA_DIR/system/lib/modules/$1
}

function create_sound_sorft_link()
{
    cd $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai/audio_resource/
    sounds=`ls`
    for i in $sounds; do
        if [ "$i" == "power_up.opus" ]; then
            continue
        fi
        rm $i
        ln -s /system/sound/$i $i
        logger_info normal_info "Create $i soft connection system"
    done
    cd -
}

function install_app_config_to_rootfs()
{
    rm -rf $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai
    mkdir -p $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai
    cp -rf $APP_CONFIG_DIR/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai/
    mv $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai/version $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/
    rm -rf $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai/script
    rm -rf $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai/model/*.bin
    cp $APP_CONFIG_DIR/script/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/init.d
    rm -rf $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/init.d/setup.sh
    rm -rf $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai/audio_resource/*
    if  [ ${ZRT_ENV_BOARD_VERSION} = "HM1002_CN" ]; then
        cp -rf $APP_CONFIG_DIR/audio_resource/chinese/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai/audio_resource
    else
        cp -rf $APP_CONFIG_DIR/audio_resource/english/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/70mai/audio_resource
    fi
    create_sound_sorft_link

    if [ $ZRT_ENV_BUILD_TYPE == debug ]; then
        cp -r $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera_debug/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/
        rm $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/bin/iperf
    fi
    cp  $TOOLS_BIN_DIR/hardware/${ZRT_ENV_HARDWARE_VERSION}/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/bin
}

function install_app_config_to_system()
{
    mkdir -p $ZRT_ENV_OUT_CAMERA_DIR/system/sound
    if  [ ${ZRT_ENV_BOARD_VERSION} = "HM1002_CN" ]; then
        cp -r $APP_CONFIG_DIR/audio_resource/chinese/* $ZRT_ENV_OUT_CAMERA_DIR/system/sound
    else
        cp -r $APP_CONFIG_DIR/audio_resource/english/* $ZRT_ENV_OUT_CAMERA_DIR/system/sound
    fi
    cp  $TOOLS_BIN_DIR/ethtool $ZRT_ENV_OUT_CAMERA_DIR/system/bin
    cp  $APP_CONFIG_DIR/version $ZRT_ENV_OUT_CAMERA_DIR/system/
    cp  $TOOLS_BIN_DIR/hardware/${ZRT_ENV_HARDWARE_VERSION}/* $ZRT_ENV_OUT_CAMERA_DIR/system/bin
}

function install_model_to_rootfs()
{
    cp $TOOLS_BIN_DIR/flash_od_model $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/bin
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/bin/flash_od_model
}

function install_uvc_to_system()
{
    set +e
    uvc_str='uvc'
    result=$(echo "${ZRT_ENV_KERNEL_CONFIG_70MAI}" | grep "${uvc_str}")
    if [ ! -z ${result} ]; then
        mkdir -p $ZRT_ENV_OUT_CAMERA_DIR/system/config
        cp $ZRT_ENV_DRIVERS_DIR/uvc_t41_demo/bin/uclibc/ucamera-t41                    $ZRT_ENV_OUT_CAMERA_DIR/system/bin
        cp $ZRT_ENV_DRIVERS_DIR/uvc_t41_demo/config/uvc.attr                           $ZRT_ENV_OUT_CAMERA_DIR/system/config
        cp $ZRT_ENV_DRIVERS_DIR/uvc_t41_demo/config/isp.config                         $ZRT_ENV_OUT_CAMERA_DIR/system/config
        cp $ZRT_ENV_DRIVERS_DIR/uvc_t41_demo/config/uvc.config_${ZRT_ENV_SENSOR}       $ZRT_ENV_OUT_CAMERA_DIR/system/config/uvc.config
        cp $ZRT_ENV_DRIVERS_DIR/uvc_t41_demo/driver/common/ucamera_driver/usbcamera.ko $ZRT_ENV_OUT_CAMERA_DIR/system/config
    fi
    set -e
}

function install_wifi_to_system()
{
    mkdir -p $ZRT_ENV_OUT_CAMERA_DIR/system/wifi
    cp $ZRT_ENV_DRIVERS_DIR/wifi/drivers/camera/${ZRT_ENV_TOOL_CHAIN}/$ZRT_ENV_WIFI_CHIP/${ZRT_ENV_LINUX_VERSION//_/.}/* $ZRT_ENV_OUT_CAMERA_DIR/system/wifi
}

function install_wifi_to_rootfs()
{
    mkdir -p $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/wifi
    cp $ZRT_ENV_DRIVERS_DIR/wifi/drivers/camera/${ZRT_ENV_TOOL_CHAIN}/$ZRT_ENV_WIFI_CHIP/${ZRT_ENV_LINUX_VERSION//_/.}/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/wifi
}

function install_mi_ot_to_system()
{
    cp -r $TOOLS_BIN_DIR/mi_ot $ZRT_ENV_OUT_CAMERA_DIR/system
    cp $TOOLS_BIN_DIR/atbm_iot_cli_status $ZRT_ENV_OUT_CAMERA_DIR/system/bin
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/system/bin/atbm_iot_cli_status
}
function install_nfs_server_to_system()
{
    cp $TOOLS_BIN_DIR/nfs_server/* $ZRT_ENV_OUT_CAMERA_DIR/system/bin
}

function install_nfs_server_to_rootfs()
{
    cp $TOOLS_BIN_DIR/nfs_server_config/* $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/etc/
}


function install_ota_6441_to_system()
{
    cp  $TOOLS_BIN_DIR/OTA_6441_zip.bin $ZRT_ENV_OUT_CAMERA_DIR/system/bin
}

if [ "$1" == "system" ]; then
    install_app_to_$1 ai_manage_subsystem
    install_app_to_$1 audio_manage_subsystem
    # install_app_to_$1 video_manage_subsystem
    install_app_to_$1 speaker_manage_subsystem
    install_app_to_$1 ipc_manage_subsystem
    install_app_to_$1 center_manage_subsystem
    install_app_to_$1 event_manage_subsystem
    install_app_to_$1 dev_manage_subsystem
    install_app_to_$1 cli_agent_client
    install_app_to_$1 log_manage_subsystem
    install_app_to_$1 ota_manage_subsystem
    # install_app_to_$1 local_storage_subsystem

    install_lib_to_$1 libimgalgo.so
    install_lib_to_$1 libmi_alg_t41_detector_sdk.so
    install_lib_to_$1 libalgsdk.so
    install_lib_to_$1 libaip.so
    install_lib_to_$1 libdrivers.so
    install_lib_to_$1 libvenus.so
    install_lib_to_$1 libCLILib.so
    install_lib_to_$1 libncurses.so.5
    # install_libffmpeg_to_$1 libavcodec.so*
    # install_libffmpeg_to_$1 libavformat.so*
    # install_libffmpeg_to_$1 libavutil.so*
    # install_libffmpeg_to_$1 libswresample.so*
    # install_libffmpeg_to_$1 libmp4v2.so*

    install_lkm_to_$1 ltr-311als.ko
    install_lkm_to_$1 als-hx3205.ko
    install_lkm_to_$1 mpsys.ko
    install_lkm_to_$1 soc-nna.ko

    install_app_config_to_system
    install_uvc_to_system
    install_mi_ot_to_system

    cp $TOOLS_BIN_DIR/jz-uvc-gadget/uvc-gadget $ZRT_ENV_OUT_CAMERA_DIR/system/bin
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/system/bin/uvc-gadget
    
    cp $TOOLS_BIN_DIR/enter_factory_auth/enter_factory_auth $ZRT_ENV_OUT_CAMERA_DIR/system/bin
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/system/bin/enter_factory_auth
    install_nfs_server_to_system
elif [ "$1" == "rootfs" ]; then
    install_app_to_$1 media_source_subsystem

    install_lib_to_$1 libcomproxy.so
    install_lib_to_$1 librtp.so
    install_lib_to_$1 libcore.so
    install_lib_to_$1 libosp.so
    install_lib_to_$1 libjson.so
    install_lib_to_$1 librpc.so
    install_lib_to_$1 libcrypt.so
    install_lib_to_$1 libcserialport.so
    install_lib_to_$1 libuuid.so.1
    install_zeratul_lib_to_$1 libaudioProcess.so
    install_zeratul_lib_to_$1 libimp.so
    install_zeratul_lib_to_$1 libalog.so
    install_libdir_to_$1 platform-media

    install_lkm_to_$1 env-flash.ko
    
    cp $TOOLS_BIN_DIR/efuse_tool/efuse_tool $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/bin
    mips-linux-gnu-strip $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera/usr/bin/efuse_tool
    install_app_config_to_rootfs
    install_wifi_to_rootfs
    install_model_to_rootfs
    install_nfs_server_to_rootfs
else 
    logger_info error_info "Please enter the rootfs or system"
fi
