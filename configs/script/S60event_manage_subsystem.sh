#/bin/sh

start()
{	
	event_manage_subsystem /etc/70mai/event_manage_subsystem/event_manage_app.ini &
	touch /var/daemon/event_manage_subsystem
}

stop()
{
	rm /var/daemon/event_manage_subsystem
	killall event_manage_subsystem	
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
