# arduino-nrf24

## Arduino NANO - Remote controller
This sketch (TX) does the following:
It sends the data array filled with buttons and potentiometers (joystick) values connected to the Arduino.

## Arduino UNO - DIY car
This sketch (RX) does the following:
It receives the data with buttons and joystick statuses by radio channel (NRF24L01) and controls the mechanical devices (servo, motor, led).

### Arduino NANO pins -> TX
---- jostik ----
+5v 		-> +5v
GND 		-> GND
X 			-> A0
Y 			-> A1
BTN 		-> D8

---- buttons ----
BTN_green 	-> D2
BTN_red 	-> D3
BTN_blue 	-> D4
BTN_yellow 	-> D5
GND 		-> GND

---- nRF24L01 ----
SCK  		-> 13
MISO 		-> 12
MOSI 		-> 11
CSN  		-> 10
CE   		-> 9
+5v			-> +5v
GND			-> GND
