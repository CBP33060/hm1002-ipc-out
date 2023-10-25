#/bin/sh

insmod_nna

insmod /lib/modules/env-flash.ko factory_mode=$FACTORY_MODE
insmod /system/lib/modules/ltr-311als.ko
insmod /system/lib/modules/als-hx3205.ko

set_wifi()
{
	while [ ! -e /usr/wifi/server_socket ]
	do
		sleep 0.1
	done
	
	bind_status=`fw_printenv user bind_status`
	if [ "$bind_status" != "ok" ] ;then
		model=`fw_printenv factory model`
		model2=${model/./-}
		model3=${model2/./-}

		envmac=`fw_printenv factory mac`
		mac1=${envmac##*:}
		mac2=${envmac%:*}
		mac3=${mac2##*:}
		ap_ssid=$model3"_miap"$mac3$mac1
		echo $ap_ssid

		cd /usr/wifi/
		./atbm_iot_cli wifi_mode AP
		./atbm_iot_cli set_ap_cfg ssid $ap_ssid
		./atbm_iot_cli set_ap_cfg key_mgmt NONE
		./atbm_iot_cli enable_ap_cfg
		cd -

		/system/mi_ot/miio_client_helper_nomqtt.sh.mijia > /dev/null 2>&1 &
        /system/mi_ot/miio_client -l 4 -d /system/mi_ot/config/ > /dev/null 2>&1 &
	else
		echo
		cd /usr/wifi
		wifi_status=`atbm_iot_cli_status wifi_mode`
		echo wifi_status:$wifi_status		


		if [ "$wifi_status" == "AP" ]; then
			./atbm_iot_cli wifi_mode STA
		fi

        lowpower_mode=`fw_printenv user lowpower_mode`
		if [ $lowpower_mode == 0 ] ;then
			fw_setenv user lowpower_mode 1
		fi
		# wifi_ssid=`fw_printenv user wifi_ssid`
		# echo $wifi_ssid
		# wifi_passwd=`fw_printenv user wifi_passwd`
		# echo $wifi_pa

		# cd /usr/wifi/
		# ./atbm_iot_cli wifi_mode STA
		# sleep 0.5

		# ./atbm_iot_cli set_network ssid $wifi_ssid

		# if [ "$wifi_passwd" ];then
		# 	./atbm_iot_cli set_network key $wifi_passwd
		# 	./atbm_iot_cli set_network key_mgmt WPA2
		# else
		# 	./atbm_iot_cli set_network key_mgmt NONE
		# fi

		# ./atbm_iot_cli enable_network
		# cd -
    fi
}

# 工厂模式
if [ $FACTORY_MODE != "0" ];then
	insmod_uvc
	uvc-gadget &
	
	if [ "$PS1" ]; then
		if [ "`id -u`" -eq 0 ]; then
			export PS1='[\u@\h:\W]# '
		else
			export PS1='[\u@\h:\W]$ '
		fi
	fi
	
	export TERMINFO=/lib/terminfo/

	/system/mi_ot/miio_client -l 4 -d /system/mi_ot/config/ > /dev/null 2>&1 &
	cli_agent_client --input /dev/ttyGS0 &
	./etc/init.d/S99daemon start &

	touch /var/daemon/cli_agent_client
else
	set_wifi &
fi
