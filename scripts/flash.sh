#!/bin/bash
dfu-programmer atmega32u4 erase && dfu-programmer atmega32u4 flash ../../../devel/share/autonomy_leds/firmware/leds_firmware.hex
