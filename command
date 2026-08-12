google-chrome-stable --disable-features=GlobalMediaControls,MediaSessionService
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
ghostty --fullscreen
gnome-terminal --full-screen
ghostty --fullscreen=true --theme=Ubuntu
gnome-terminal --full-screen --show-menubar
Set-PSReadLineKeyHandler -Key Tab -Function AcceptSuggestion
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
scp target/Treds-BiddingPortal.jar dipu@103.25.172.132:/opt/UAT/biddingPortal/
scp dipu@103.25.172.132:/opt/UAT/biddingPortal_logs/rolling/portal-20-05-2026-0.log .
scp dipu@103.25.172.163:/opt/STAGING/biddingPortal_logs/rolling/portal-01-06-2026-0.log .
scp dipu@10.255.74.14:/opt/STAGING/biddingPortal_logs/rolling/portal-02-07-2026-0.log .
rsync --rsync-path="sudo rsync" target/Treds-BiddingPortal.jar dipu@103.25.172.132:/opt/UAT/biddingPortal/
curl -u configuser:Config@Secret123 -ks https://uat.m1xchange.com:8477/M1-BiddingPortal/qa
curl -u configuser:Config@Secret123 -ks https://uat.m1xchange.com:8477/M1-DocumentService/dev
curl -u configuser:Config@Secret123 -ks https://uat.m1xchange.com:8477/M1-DocumentService/dev
===========================================================================================================================
grep -n uploadManageBulkInvoices portal-22-06-2026-0.log|tail -1|cut -d: -f1|xargs -I{} tail -n +{} portal-22-06-2026-0.log
grep -n saveRiskRpRequest portal-08-05-2026-1.log|tail -1|cut -d: -f1|xargs -I{} tail -n +{} portal-08-05-2026-1.log|less
grep REQUEST portal-08-05-2026-1.log|grep -n saveRiskRpRequest|tail -1
grep -an "https-jsse-nio-8449-exec-4.*REQUEST.*uploadManageBulkInvoices" portal-22-06-2026-0.log
grep -nF 'getBillingPopupStatus/phaseoutpopup' portal-08-05-2026-1.log|tail -1|cut -d: -f1|xargs -I{} tail -n +{} portal-08-05-2026-1.log|less
grep -nF 'whatsNewDocument/302' portal-19-05-2026-0.log|tail -1|cut -d: -f1|xargs -I{} tail -n +{} portal-19-05-2026-0.log|less
grep REQUEST portal-19-05-2026-0.log|grep -nF 'whatsNewDocument/302'|tail -1|cut -d: -f1
grep -nF -e 'REQUEST' -e 'whatsNewDocument/302' portal-19-05-2026-0.log|tail -1|cut -d: -f1
grep -nF 'REQUEST' portal-19-05-2026-0.log|grep -F 'whatsNewDocument/302'|tail -1|cut -d: -f1|xargs -I{} tail -n +{} portal-19-05-2026-0.log|less
=================================================================================================================================================
grep -nF 'REQUEST' portal-20-05-2026-0.log|grep -F 'Common/getTdsBillingDataMonthwise'|tail -1
grep -nF 'REQUEST' portal-20-05-2026-0.log|grep -F 'Common/getTdsBillingDataMonthwise'|tail -1|cut -d: -f1
==========================================================================================================
find . -type f -name "*.java" | wc -l
claude "Run full pipeline for M1BPW-602"
svn resolve --accept theirs-full filename
rpm -qf /usr/lib64/evolution-data-server/calendar-backends/libecalbackendweather.so
find /path/to/search -type d -name "node_modules" -exec rm -rf {} +
find /path/to/search -name "target_name" -exec rm -rf {} +
find /path/to/search -type f -name "*.log" -delete
locate -0 -i anthy | sudo xargs -0 rm -rvf
locate -i baobab|sudo xargs -d '\n' rm -rvf
