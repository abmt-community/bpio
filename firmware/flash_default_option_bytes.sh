#!/bin/bash

st-flash --reset write release/option_bytes_default.bin 0x1FFFF800
