/*
  esp8262 with LM35 and LED Monitored & contolled using MQTT
  
  This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
  
  This Project aimed to apply simple IoT priciples for beginners by using easy to use elements NodeMCU and Adafruit io

  The circuit:
  * NodeMCU v1.0
  * LED
  * LM35

  Created 19 March 2019
  By Mujahed Altahle

*/

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "IOT-WiFi"
#define WLAN_PASS       "IOT@9922"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME  "mtahle"
#define AIO_KEY       "5f9cb21169754ddfb2b623d7a45b1c5e"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called 'temp' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe ledOnOff = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** Sketch Code ************************************/
void MQTT_connect();
float tempC;
int tempPin = A0;
int ledPin = LED_BUILTIN;
void setup()
{

  Serial.begin(115200);
  delay(10);
  pinMode(ledPin, OUTPUT);
  
  Serial.println(F("Wasslz MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&ledOnOff);
}


float readTemp()
{
  int analogValue = analogRead(tempPin);
  float millivolts = (analogValue / 1024.0) * 3300; //3300 is the voltage provided by NodeMCU
  float celsius = millivolts / 10;
  return celsius;
}
void loop()
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {  //mqtt.readSubscription(timeInMilliseconds)
    if (subscription == &ledOnOff) {
      Serial.print(F("Got: "));
      Serial.println((char *)ledOnOff.lastread);
      String receivedValue = (char *)ledOnOff.lastread;
      if (receivedValue == "ON")
      {
        digitalWrite(ledPin, LOW);
      }
      else
      {
        digitalWrite(ledPin, HIGH);
      }
    }
  }


  tempC = readTemp();
  //Serial.print("Temperature is= ");
  //Serial.println(tempC);
  // Now we can publish stuff!
  Serial.print(F("\nSending temp val "));
  Serial.print(tempC);
  Serial.print("...");
  if (! temp.publish(tempC)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  delay(1000);
}
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
