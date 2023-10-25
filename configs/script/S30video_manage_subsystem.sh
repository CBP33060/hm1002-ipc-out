#/bin/sh

start()
{	
	# video_manage_subsystem /etc/70mai/video_manage_subsystem/video_manage_app.ini &
	# touch /var/daemon/video_manage_subsystem
	echo
}

stop()
{
	# rm /var/daemon/video_manage_subsystem
	# killall video_manage_subsystem	
	echo
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
