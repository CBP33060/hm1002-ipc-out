#/bin/sh

start()
{	
	log_manage_subsystem /etc/70mai/log_manage_subsystem/log_manage_app.ini &
	touch /var/daemon/log_manage_subsystem
}

stop()
{
	rm /var/daemon/log_manage_subsystem
	killall log_manage_subsystem	
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
