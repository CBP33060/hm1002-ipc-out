#/bin/sh

start()
{	
	center_manage_subsystem /etc/70mai/center_manage_subsystem/center_manage_app.ini &
	touch /var/daemon/center_manage_subsystem
}

stop()
{
	rm /var/daemon/center_manage_subsystem
	killall center_manage_subsystem	
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
