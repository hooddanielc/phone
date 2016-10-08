set -e
ib phone-controller
avr-objcopy -O ihex -R .eeprom ../out/debug/phone-controller ../out/debug/phone-controller.hex
