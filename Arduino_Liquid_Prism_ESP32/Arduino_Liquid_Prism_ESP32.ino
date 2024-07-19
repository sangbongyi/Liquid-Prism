/* 
  Liquid prism project
  Copyright (c) 2024 Sangbong Lee <sangbong@me.com>
  
  * ESP32 board with three light sensors for laser. 
  * This code allows the custom-designed esp32 board to get values from scattered laser light in the test tube. 
  * The data from the board is transmitted through WiFi to the sound system.

  This work is licensed under the Creative Commons Attribution 4.0 International License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/

// Include Libraries
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

// Set Potentiometer Pins
const int PT_pin_1 = 34;
const int PT_pin_2 = 35;
const int PT_pin_3 = 32;

// Set LED Pins
const int LED_WIFI = 23;
const int LED_SEND = 22;

// Set Potentiometer values
int PT_val_1 = 0;
int PT_val_2 = 0;
int PT_val_3 = 0;

// Set LED Indicator flag
unsigned int WiFi_LED_FLAG = LOW;
unsigned int SEND_LED_FLAG = LOW;

// WiFi Credentials
char ssid[] = "CHC";         // network SSID (name)
char pass[] = "chihimchik";  // network password
// WIFI UDP
WiFiUDP Udp;
// WIFI Address
const IPAddress outIp(10, 86, 86, 86);

// Port info
const unsigned int outPort = 4444;    // Send Port
const unsigned int localPort = 8888;  // Receive Port 8,7,6,5,4,3,2,1

// OSC
OSCErrorCode error;

void setup() {
  /* Setup LEDs */
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_SEND, OUTPUT);

  /* Setup WiFi */
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    WiFi_LED_FLAG = LOW;
  }
  WiFi_LED_FLAG = HIGH;

  /* Setup UDP */
  Udp.begin(localPort);

  /* Setup OTA functions */
  ArduinoOTA.setHostname("PMD_Module_8");
  ArduinoOTA.begin();
}

void loop() {
  /* OTA update handle */
  ArduinoOTA.handle();

  /* Set WIFI Indicator LEDs */
  digitalWrite(LED_WIFI, WiFi_LED_FLAG);

  /* OSC message instance */
  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/led_8", LED_OSC);
    } else {
      error = msg.getError();
    }
  }

  /* Get Photo transistor values */
  PT_val_1 = analogRead(PT_pin_1);
  PT_val_2 = analogRead(PT_pin_2);
  PT_val_3 = analogRead(PT_pin_3);

  /* Send Photo transistor values */
  if (WiFi.status() == WL_CONNECTED) {

    OSCMessage msg_1("/MD8_sensors");
    float toSend_1 = PT_val_1;
    float toSend_2 = PT_val_2;
    float toSend_3 = PT_val_3;

    msg_1.add(toSend_1);
    msg_1.add(toSend_2);
    msg_1.add(toSend_3);

    // Send UDP packet and flush the message
    Udp.beginPacket(outIp, outPort);
    msg_1.send(Udp);
    Udp.endPacket();
    msg_1.empty();
  }
}

/* Get Message LED indicator values */
void LED_OSC(OSCMessage &msg) {
  SEND_LED_FLAG = msg.getInt(0);
  digitalWrite(LED_SEND, SEND_LED_FLAG);
}
