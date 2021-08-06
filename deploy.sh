#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD
echo -e "\n\nInstalling commons libraries...\n\n"
COMMONS="so-commons-library"
sudo rm -f -r so-commons-library
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD
echo -e "\n\nInstalling gui-library...\n\n"
sudo rm -f -r so-nivel-gui-library
git clone "https://github.com/sisoputnfrba/so-nivel-gui-library/"
cd so-nivel-gui-library
sudo make uninstall
sudo make all
sudo make install
cd $CWD
echo -e "\n\nBuilding projects...\n\n"
make -C discordiador
make -C miramhq
make release -C imongostore
echo -e "\n\nDeploy done!\n\n"
