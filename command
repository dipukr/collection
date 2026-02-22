sudo dd bs=4M if=archlinux-2025.01.01-x86_64.iso of=/dev/sda conv=fsync oflag=direct status=progress
mvn archetype:generate -DarchetypeArtifactId=maven-archetype-quickstart -DarchetypeVersion=1.1
sqlite3 cookies.sqlite "SELECT host, name, value FROM moz_cookies;"
alter user 'root'@'localhost' identified by 'P@55Word';
python3.12 -m ensurepip:python3.12 -m pip list
git remote add origin url.git:git push -uf origin master
g++ -c add.cpp:ar crf ..\lib\libadd.a add.o:g++ -o main main.cpp -L. -ladd
g++ -c -O3 -std=c++11 -I"../include"
jar cfe Main.jar Main *.class
g++ -c -std=c++11 -I"/usr/include/qt5/" qt.cxx
g++ -std=c++17 -I"/usr/include/qt6/" qt.cxx /usr/lib64/libQt6Core.so -o qt
firewall-cmd --permanent --add-port=1234/tcp:firewall-cmd --reload
dconf reset /org/gnome/shell/command-history
python3.12 -m pip install --upgrade pip
journalctl -u nginx -f --no-pager
nmap -n -PN -sT -p- localhost
ss -tulnp|grep :8780
=============================================================================
gsettings set org.gnome.shell.extensions.dash-to-dock click-action 'minimize'
gsettings set org.gnome.Terminal.Legacy.Settings default-show-menubar true
gsettings set org.gnome.Terminal.Legacy.Settings headerbar false
gsettings set org.gnome.mutter center-new-windows true
git config --global user.name "dipukr"
git config --global user.email "adipukr@gmail.com"
git config --global core.editor "vim"
git config --global core.excludesfile ~/.config/.gitignore
git config --global credential.helper "/bin/bash /home/dkumar/.config/.git-credentials"
=============================================================================================================
zookeeper-server-start /opt/kafka/zookeeper.properties:kafka-server-start /opt/kafka/config/server.properties
kafka-topics --bootstrap-server localhost:9092 --create --topic topic0 --partitions 3 --replication-factor 1
kafka-topics --bootstrap-server localhost:9092 --list
kafka-topics --bootstrap-server localhost:9092 --describe --topic topic0
kafka-console-producer --broker-list localhost:9092 --topic topic0
kafka-console-producer --broker-list localhost:9092 --topic topic0 < bin/customers.csv
kafka-console-consumer --bootstrap-server localhost:9092 --topic topic0 --from-beginning
===============================================================================================================
kafka-topics.sh --create --topic topic0 --partitions 3 --replication-factor 1 --bootstrap-server localhost:9092
kafka-topics.sh --create --topic quickstart-events --bootstrap-server localhost:9092
kafka-topics.sh --describe --topic quickstart-events --bootstrap-server localhost:9092
kafka-topics.sh --describe --topic topic0 --bootstrap-server localhost:9092
kafka-topics.sh --list --bootstrap-server localhost:9092
kafka-console-producer.sh --topic quickstart-events --bootstrap-server localhost:9092
kafka-console-producer.sh --broker-list localhost:9092 --topic topic0 < bin/customers.csv
kafka-console-consumer.sh --topic quickstart-events --from-beginning --bootstrap-server localhost:9092
======================================================================================================
gnome-terminal --full-screen --show-menubar
ghostty --fullscreen=true --theme=Ubuntu
gnome-terminal --full-screen
ghostty --fullscreen


