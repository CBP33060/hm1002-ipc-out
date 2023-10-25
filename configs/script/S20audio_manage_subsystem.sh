#/bin/sh

start()
{	
	audio_manage_subsystem /etc/70mai/audio_manage_subsystem/audio_manage_app.ini &
	touch /var/daemon/audio_manage_subsystem
}

stop()
{
	rm /var/daemon/audio_manage_subsystem
	killall audio_manage_subsystem	
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
