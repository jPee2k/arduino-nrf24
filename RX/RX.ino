/*
 * Arduino UNO - DIY car
 * This sketch (RX) does the following:
 * It receives the data with buttons and joystick statuses by radio channel (NRF24L01)
 * and controls the mechanical devices (servo, motor, led).
 */

#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <Servo.h>

Servo servo_motor;

/*
 * Contacts from the radio module NRF24L01 connection to pins -> Arduino UNO
 * SCK  -> 13
 * MISO -> 12
 * MOSI -> 11
 * CSN  -> 10
 * CE   -> 9
 * /

/* Contacts from a motor driver connected to pins -> Arduino UNO */
#define D1 4 // D1 pin 4
#define D0 5 // D0 pin 5

#define LED_1 7
#define pin_servo 2
#define calibration_angle 9

/* Initialize te radio module on the pins 9 & 10 -> UNO */
RF24 radio(9, 10);

/* Radio module pipes (select one of) */
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};

int transmitted_data[2][5];

void setup()
{
  Serial.begin(9600); // open console port

  pinMode(LED_1, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D0, OUTPUT);

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

  radio.setAutoAck(1);                  // acknowledgment mode, 1 on 0 off
  radio.setRetries(0, 15);              // the time between attempts to reach, number of attempts
  radio.enableAckPayload();             // allow sending data in response to an incoming signal
  radio.setPayloadSize(16);             // packet size, in bytes | default 32
  radio.openReadingPipe(1, address[0]); // open a pipe channel on pipe 0
  radio.setChannel(0x6e);               // choose channel (withoul noises!)

  /* must be the same on the receiver and transmitter!
   * at the lowest speed is the highest sensitivity and range!!
   */
  radio.setPALevel(RF24_PA_MAX);   // transmitter power level -> RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_250KBPS); // exchange speed -> RF24_2MBPS, RF24_1MBPS, RF24_250KBPS

  radio.powerUp();        // module power up
  radio.startListening(); // listen to the radio, it is a receiver

  servo_motor.attach(pin_servo);
  servo_motor.write(90 - calibration_angle);
}

/* Used inverted values for the engine */
void engine_handler(boolean green_btn, boolean red_btn, boolean blue_btn, boolean yellow_btn)
{
  /* brake:    D1 - HIGH,  D0 - HIGH
   * idle:     D1 - LOW,   D0 - LOW
   * forward:  D1 - LOW,   D0 - HIGH / PWM
   * backward: D1 - HIGH,  D0 - LOW  / PWM
   */

  /* idle */
  if (!green_btn && !red_btn && !blue_btn && !yellow_btn)
  {
    digitalWrite(D1, LOW);
    digitalWrite(D0, LOW);
  }

  /* forward */
  if (green_btn)
  {
    digitalWrite(D1, HIGH);
    analogWrite(D0, -255);
  }

  /* backward */
  if (red_btn)
  {
    digitalWrite(D1, LOW);
    analogWrite(D0, 255);
  }

  /* brake */
  if (blue_btn)
  {
    digitalWrite(D1, HIGH);
    digitalWrite(D0, HIGH);
  }

  /* slow forward */
  if (yellow_btn)
  {
    digitalWrite(D1, HIGH);
    analogWrite(D0, -127);
  }
}

void run_app()
{
  byte joystick_btn = transmitted_data[0][4];
  int joystick_val_x = transmitted_data[1][0];
  int joystick_val_y = transmitted_data[1][1];

  /* handle servo */
  int servo_angle = map(joystick_val_x, -255, 255, 0, 180 - calibration_angle);
  if (joystick_val_x == 0)
  {
    servo_motor.write(90 - calibration_angle);
  }
  else
  {
    servo_motor.write(servo_angle);
  }

  /* handle led */
  digitalWrite(LED_1, joystick_btn); // toggle led

  /* handle engine */
  engine_handler(
      transmitted_data[0][0], // green_btn
      transmitted_data[0][1], // red_btn,
      transmitted_data[0][2], // blue_btn,
      transmitted_data[0][3], // yellow_btn
  );
}

void loop()
{
  if (radio.available())
  {
    radio.read(&transmitted_data, sizeof(transmitted_data));
    run_app();
  }
}
