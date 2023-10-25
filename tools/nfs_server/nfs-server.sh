#!/bin/sh


[ -x /system/bin/portmap ] || exit 0
[ -x /system/bin/rpc.statd ] || exit 0
[ -x /system/bin/rpc.nfsd ] || exit 0
[ -x /system/bin/rpc.mountd ] || exit 0
[ -x /system/bin/exportfs ] || exit 0

mkdir -p /var/lib/nfs/sm
mkdir -p /var/lib/nfs/sm.bak
mkdir -p /var/lock/subsys
mkdir -p /userfs/nfs
touch /var/lib/nfs/etab
touch /var/lib/nfs/rmtab
touch /var/lib/nfs/state
touch /var/lib/nfs/xtab


start() {
	# Start daemons.
	echo -n "Starting port mapper: "
	portmap                         
	echo "done"                     
 
	echo -n "Starting NFS statd: "
	rpc.statd
	touch /var/lock/subsys/nfslock
	echo "done"
 
	echo -n "Starting NFS services: "
	exportfs -r
	rpc.statd
	echo "done"
 
	echo -n "Starting NFS daemon: "
	rpc.nfsd 2
	echo "done"
 
	echo -n "Starting NFS mountd: "
	rpc.mountd
	echo "done"
	touch /var/lock/subsys/nfs
}

stop() {
	# Stop daemons.
	echo -n "Shutting down NFS mountd: "
	killall -q rpc.mountd
	echo "done"
 
	echo "Shutting down NFS daemon: "
	kill -9 `pidof nfsd` 2>/dev/null
	echo "done"
 
	echo -n "Shutting down NFS services: "
	exportfs -au
	rm -f /var/lock/subsys/nfs
	killall -q rpc.statd
	echo "done"
 
	echo -n "Stopping NFS statd: "
	killall -q rpc.statd
	echo "done"
	rm -f /var/lock/subsys/nfslock
 
	echo -n "Stopping port mapper: "
	killall -q portmap
	echo "done"
}


case "$1" in
  start)
	start
	;;
  stop)
	stop
	;;
  *)
	echo "Usage: nfs {start|stop}"
	exit 1
esac
 
exit 0
