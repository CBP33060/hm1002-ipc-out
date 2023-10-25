#/bin/sh

cmdline=`cat /proc/cmdline`
__senv=${cmdline#*senv;\[HW\];}
_senv=${__senv%;eenv*}
senv=${_senv//;/ }
echo $senv

_factory_mode=${senv#*70mai_factory_mode=}
factory_mode=${_factory_mode%% *}
echo factory_mode:$factory_mode

_system_partition=${senv#*70mai_system_partition=}
system_partition=${_system_partition%% *}
echo system_partition:$system_partition

export SYSTEM_PARTITION=$system_partition
export FACTORY_MODE=$factory_mode
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib/platform-media/

mkdir -p /var/daemon

# 正常模式
if [ $FACTORY_MODE == "0" ]; then
    media_source_subsystem /etc/70mai/media_source_subsystem/media_source_app.ini &
    touch /var/daemon/media_source_subsystem
fi

mkdir -p /tmp/70mai

insmod_mmc
/usr/wifi/wifi_start_work.sh /usr/wifi