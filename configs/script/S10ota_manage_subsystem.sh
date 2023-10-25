#/bin/sh

start()
{	
	ota_manage_subsystem /etc/70mai/ota_manage_subsystem/ota_manage_app.ini &
	touch /var/daemon/ota_manage_subsystem
}

stop()
{
	rm /var/daemon/ota_manage_subsystem
	killall ota_manage_subsystem	
}

case "$1" in
	start)
		"$1"
		;;
	stop)
		"$1"
		;;
	*)
		echo "Usage: $0 {start}"
		exit 1
		;;
esac
