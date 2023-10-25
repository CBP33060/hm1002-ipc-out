#/bin/sh

start()
{	
	speaker_manage_subsystem /etc/70mai/speaker_manage_subsystem/speaker_manage_app.ini &
	touch /var/daemon/speaker_manage_subsystem
}

stop()
{
	rm /var/daemon/speaker_manage_subsystem
	killall speaker_manage_subsystem	
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
