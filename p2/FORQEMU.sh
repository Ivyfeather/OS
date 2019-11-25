make all
dd if=image of=disk conv=notrunc
cd /home/stu/QEMULoongson/
sh run_pmon2.sh
cd /mnt/shared/p2
