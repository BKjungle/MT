#/bin/bash

:<<\EOF
ssxxxxxx
EOF


for x in file_server http_msg_server  push_server login_server db_proxy_server msg_server route_server 
do
	cd $x 
	if [ -e $s ]
		then
			pid=`cat server.pid`
		echo "kill $x  pid=$pid"
		kill $pid
	fi
	cd ..
done
#:<<\EOF
msfs=`ps aux|grep msfs | awk '{print $2}'|sed -n '1p'`
kill $msfs
echo "kill msfs pid=$msfs"
#EOF
