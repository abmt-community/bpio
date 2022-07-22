#!/bin/bash

#mbed compile && st-flash --flash=128k write BUILD/BLUE_PILL/GCC_ARM/firmware.bin 0x8000000
for i in 1 2 3 4 0; do
	sed -i main.cpp -e "s/DEVICE_NUM .*/DEVICE_NUM '$i'/"
	mbed compile
	cp BUILD/BLUE_PILL/GCC_ARM/firmware.bin release/bpio_fw_id$i.bin
done
