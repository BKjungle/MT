#/bin/sh



for x in base file_server http_msg_server  push_server login_server db_proxy_server msg_server route_server   clientRobot_EVP  msfs delfile 
do
    cd $x

	rm CMakeFiles CMakeCache.txt cmake_install.cmake  Makefile -rf

    cd ..
done
echo all is ok
