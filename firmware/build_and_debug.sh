#!/bin/bash

#mbed compile && st-flash --flash=128k write BUILD/BLUE_PILL/GCC_ARM/firmware.bin 0x8000000
mbed compile --profile ./mbed-os/tools/profiles/debug.json  && st-flash --flash=128k write BUILD/BLUE_PILL/GCC_ARM-DEBUG/firmware.bin 0x8000000 && st-util
