#!/bin/sh

echo "Preparing for writing to SD Card.\nPlease ensure that SD Card is plugged and is not locked."
rm -rf /tmp/sdimage.img

echo "Converting $1 to v210 iROM SD Card format"
arm-elf-objcopy -O binary $1 /tmp/sdimage.bin
./tools/mkv210_image/build/mkv210_image /tmp/sdimage.bin /tmp/sdimage.img

echo "un-mounting SD Card to be able to format"
diskutil unmountDisk /dev/disk3

echo "Writing to SD Card"
sudo dd bs=512 if=/tmp/sdimage.img of=/dev/disk2 seek=1

echo "un-mounting SD Card"
diskutil unmountDisk /dev/disk2

echo "Cleaning up"
rm -rf /tmp/sdimage.bin
rm -rf /tmp/sdimage.img

echo "Now it is safe to unplug the SD Card"
