ARDUINO_PORT=$1
ARDUINO_BOARD=ATMEGA328P
avrdude -F -V -c arduino -p $ARDUINO_BOARD -P $ARDUINO_PORT -b 115200 -U flash:w:../out/debug/phone-controller.hex
