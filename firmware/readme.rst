====
BPIO
====

Flash precompiled firmware
==========================
- Get and install stlink utils from https://github.com/stlink-org/stlink
- Unlock device: 'st-flash --reset write release/option_bytes_default.bin 0x1FFFF800'
- Flash firmware 'st-flash write release/bpio_fw_id0.bin 0x8000000'

udev rules for all ID's
=======================
SUBSYSTEM=="tty", ATTRS{idVendor}=="1f00", ATTRS{idProduct}=="2012", ATTRS{product} == "BPIO0", SYMLINK+="ttyBPIO0"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1f00", ATTRS{idProduct}=="2012", ATTRS{product} == "BPIO1", SYMLINK+="ttyBPIO1"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1f00", ATTRS{idProduct}=="2012", ATTRS{product} == "BPIO2", SYMLINK+="ttyBPIO2"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1f00", ATTRS{idProduct}=="2012", ATTRS{product} == "BPIO3", SYMLINK+="ttyBPIO3"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1f00", ATTRS{idProduct}=="2012", ATTRS{product} == "BPIO4", SYMLINK+="ttyBPIO4"

Compile and Flash
=================
Install mbed
------------
- see https://os.mbed.com/docs/mbed-os/v6.15/build-tools/install-and-set-up.html
- pip install mbed-cli

Copy mbed-os
------------
- Download release https://os.mbed.com/mbed-os/releases/
- Move contained folder to 'mbed-os'

Compile
-------
mbed compile --target BLUE_PILL --toolchain GCC_ARM

Flash option bytes
------------------
- Only for (new) locked devices
- You may have to update st-flash (see Kown Issues).
- call ./flash_default_option_bytes.sh

Flash
-----
st-flash write BUILD/BLUE_PILL/GCC_ARM/firmware.bin 0x8000000

Kown Issues
============
- Modemmanager blocks device -> uninstall usless deamon
- BluePill cant be flashed
  Many new BluePills ar'e locked and the option bytes need to be changed to flash via st-link.
  You can call 'flash_default_option_bytes.sh'
  You need a quite new version stlink from https://github.com/stlink-org/stlink. 
  Debians current version is to old. "v1.7.0-201-g254a525" worked for me.


I2C
===
You can read max 128 Bytes per request (sizeof(i2c_response)).
Delay per request is around 2ms with steppers enabled and 1ms without.
