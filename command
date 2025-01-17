git remote add origin  https://koomar_dipu@bitbucket.org/koomardipu/neem.git
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
sudo dd bs=4M if=archlinux-2025.01.01-x86_64.iso of=/dev/sda conv=fsync oflag=direct status=progress