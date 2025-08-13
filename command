sudo dd bs=4M if=archlinux-2025.01.01-x86_64.iso of=/dev/sda conv=fsync oflag=direct status=progress
git remote add origin https://koomar_dipu@bitbucket.org/koomardipu/neem.git
git push -u -f origin master
g++ -c add.cpp:ar crf ..\lib\libadd.a add.o:g++ -o main main.cpp -L. -ladd
g++ -c -O3 -std=c++11 -I"../include"
jar cfe Main.jar Main *.class
g++ -c -std=c++11 -I"/usr/include/qt5/" qt.cxx
g++ -std=c++17 -I"/usr/include/qt6/" qt.cxx /usr/lib64/libQt6Core.so -o qt
firewall-cmd --permanent --add-port=1234/tcp
firewall-cmd --reload
nmap -n -PN -sT -p- localhost
sudo ufw allow from 172.16.4.134 to any port 22
sudo ufw allow from 172.16.4.134 to any port 1729
python3.12 -m pip install --upgrade pip
sudo yum erase $(rpm -qa | grep mongodb-org)
ss -tulnp|grep :8780
journalctl -u server -f --no-pager
====================================================================
zookeeper-server-start /opt/confluent/etc/kafka/zookeeper.properties
kafka-server-start /opt/confluent/etc/kafka/server.properties
kafka-topics --bootstrap-server localhost:9092 --create --topic topic0 --partitions 3 --replication-factor 1
kafka-topics --bootstrap-server localhost:9092 --list
kafka-topics --bootstrap-server localhost:9092 --describe --topic topic0
kafka-console-producer --broker-list localhost:9092 --topic topic0
kafka-console-producer --broker-list localhost:9092 --topic topic0 < bin/customers.csv
kafka-console-consumer --bootstrap-server localhost:9092 --topic topic0 --from-beginning
========================================================================================
docker ps
docker images
docker build -t docker-sch-srv:0.1.RELEASE .
docker run -p 8780:8780 docker-sch-srv:0.1.RELEASE
==================================================

