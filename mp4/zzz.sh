#!~/Documents/MPs/mp2/zzz.sh
sudo make clean
sudo make
./copykernel.sh
bochs -f bochsrc.bxrc

