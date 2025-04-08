/*
  setup.ino helps you get your DevEUI
*/

#include <MKRWAN.h>
#include <ArduinoLowPower.h>
#include "arduino_secrets.h"
using namespace std;

LoRaModem modem;

// Uncomment if using the Murata chip as a module
// LoRaModem modem(Serial1);

void setup() {
  delay(5000); //allow some downtime to upload a new sketch
  pinMode(LED_BUILTIN, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(115200); // Initialize Serial for debuging purposes.
  while (!Serial);
  // change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(US915)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  delay(5000);
}

void loop() {}