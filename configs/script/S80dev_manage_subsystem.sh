#/bin/sh

start()
{	
	dev_manage_subsystem /etc/70mai/dev_manage_subsystem/dev_manage_app.ini &
	touch /var/daemon/dev_manage_subsystem
}

stop()
{
	rm /var/daemon/dev_manage_subsystem
	killall dev_manage_subsystem	
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
