#/bin/sh
#start or stop the im-server

IM_SERVER=./restart.sh   ##?????????

$IM_SERVER db_proxy_server
sleep 1;
$IM_SERVER file_server
sleep 1;
$IM_SERVER msg_server
sleep 1;
$IM_SERVER route_server
sleep 1;
$IM_SERVER login_server
sleep 1;
$IM_SERVER http_msg_server
sleep 1;
$IM_SERVER push_server
sleep 1;
$IM_SERVER msfs
sleep 3;
$IM_SERVER clientRobot_EVP
sleep 2;
$IM_SERVER websocket_server
