#!/bin/bash

#mbed compile && st-flash --flash=128k write BUILD/BLUE_PILL/GCC_ARM/firmware.bin 0x8000000
mbed compile && st-flash write BUILD/BLUE_PILL/GCC_ARM/firmware.bin 0x8000000
