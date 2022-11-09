/*
 * Arduino NANO - Remote controller
 * This sketch (TX) does the following:
 * It sends the data array filled with buttons and potentiometers (joystick) values connected to the Arduino.
 */

#include <SPI.h>      // library for working with the SPI bus
#include <RF24.h>     // radio module library
#include <nRF24L01.h> // radio module library L01
#include <Servo.h>    // Servo library

/* radio module pins NRF24L01 -> Arduino
 * SCK   -> 13
 * MISO  -> 12
 * MOSI  -> 11
 * CSN   -> 10
 * CE    -> 9
 */

/* joystick module pins -> Arduino
 * X     -> A0
 * Y     -> A1
 * BTN   -> D8
 */

/* buttons modeles pins -> Arduino
 * BTN_green   -> D2
 * BTN_red     -> D3
 * BTN_blue    -> D4
 * BTN_yellow  -> D5
 */

/* Initialize te radio module on the pins 9 & 10 -> NANO */
RF24 radio(9, 10);

/* Radio module pipes (select one of) */
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};

/* define pins */
int pin_y = A0;
int pin_x = A1;

byte pins[5] = {
    2, // pin_btn_green
    3, // pin_btn_red
    4, // pin_btn_blue
    5, // pin_btn_yellow
    8  // pin_btn_joystick
};

/* the output joystick value should be 255 */
#define joystick_setup_value 328
#define joystick_correct_value 255

unsigned int last_timestamp_btn_press[5]; // time of the last cklick button | remove fantom clicks (button bounce)
boolean last_buttons_state[5];            // the last buttons state [green, red, blue, yellow, joystick]
boolean isChanged = false;

int last_styck_val_x = 0;
int last_styck_val_y = 0;

int transmitted_data[2][5];

void setup()
{
  Serial.begin(9600); // open console port

  // setup btn pins
  for (byte i = 0; i < sizeof(pins); i++)
  {
    pinMode(pins[i], INPUT_PULLUP);
  }
  pinMode(pin_x, INPUT);
  pinMode(pin_y, INPUT);

  radio.begin(); // activate the module

  /* Send a module initialization notification to the console */
  Serial.println("Connected");
  if (radio.isPVariant())
  {
    Serial.println("_nRF24L01");
  }
  else
  {
    Serial.println("_unknown module");
  }

  radio.setAutoAck(1);      // acknowledgment mode, 1 on 0 off
  radio.setRetries(0, 15);  // the time between attempts to reach, number of attempts
  radio.enableAckPayload(); // allow sending data in response to an incoming signal
  radio.setPayloadSize(16); // packet size, in bytes | default 32

  radio.openWritingPipe(address[0]); // open a pipe channel on pipe 0
  radio.setChannel(0x6e);            // choose channel (withoul noises!)

  /* must be the same on the receiver and transmitter!
   * at the lowest speed is the highest sensitivity and range!!
   */
  radio.setPALevel(RF24_PA_HIGH);  // transmitter power level -> RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_250KBPS); // exchange speed -> RF24_2MBPS, RF24_1MBPS, RF24_250KBPS

  radio.stopListening(); // don't listen to the radio, it is a transmitter
}

void trim_value(int value, int &new_value)
{
  if (value > joystick_correct_value)
  {
    new_value = joystick_correct_value;
  }
  else if (value < -joystick_correct_value)
  {
    new_value = -joystick_correct_value;
  }
  else
  {
    new_value = value;
  }
}

void joystick_handler()
{
  int brutto_value_x = map(analogRead(pin_x), 503, 1023, 0, joystick_setup_value);
  int brutto_value_y = map(analogRead(pin_y), 511, 1023, 0, joystick_setup_value);

  trim_value(brutto_value_x, transmitted_data[1][0]);
  trim_value(brutto_value_y, transmitted_data[1][1]);
}

void loop()
{
  joystick_handler();

  /* set buttuns flag to the state */
  for (byte i = 0; i < sizeof(pins); i++)
  {
    /* transmitted_data[0][i] == current_button_state */
    transmitted_data[0][i] = !digitalRead(pins[i]) == HIGH;

    if (transmitted_data[0][i] != last_buttons_state[i] && millis() - last_timestamp_btn_press[i] > 60)
    {
      isChanged = true;
      last_buttons_state[i] = transmitted_data[0][i];
      last_timestamp_btn_press[i] = millis();
    }
  }

  /* Send data by a radio channel when the buttons state is changed */
  if (isChanged)
  {
    isChanged = false;

    radio.powerUp();
    radio.write(&transmitted_data, sizeof(transmitted_data));
    radio.powerDown();
  }

  /* Send data by a radio channel when the joystick state is changed */
  int styck_val_x = transmitted_data[1][0];
  if (last_styck_val_x + 2 < styck_val_x || last_styck_val_x - 2 > styck_val_x)
  {
    last_styck_val_x = styck_val_x;

    radio.powerUp();
    radio.write(&transmitted_data, sizeof(transmitted_data));
    radio.powerDown();
  }

  int styck_val_y = transmitted_data[1][1];
  if (last_styck_val_y + 2 < styck_val_y || last_styck_val_y - 2 > styck_val_y)
  {
    last_styck_val_y = styck_val_y;

    radio.powerUp();
    radio.write(&transmitted_data, sizeof(transmitted_data));
    radio.powerDown();
  }
}
