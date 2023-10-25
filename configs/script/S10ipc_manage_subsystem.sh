#/bin/sh

start()
{	
	ipc_manage_subsystem /etc/70mai/ipc_manage_subsystem/ipc_manage_app.ini &
	touch /var/daemon/ipc_manage_subsystem
}

stop()
{
	rm /var/daemon/ipc_manage_subsystem
	killall ipc_manage_subsystem	
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
