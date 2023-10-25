#/bin/sh

start()
{	
	ai_manage_subsystem /etc/70mai/ai_manage_subsystem/ai_manage_app.ini &
	touch /var/daemon/ai_manage_subsystem
}

stop()
{
	rm /var/daemon/ai_manage_subsystem
	killall ai_manage_subsystem
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
