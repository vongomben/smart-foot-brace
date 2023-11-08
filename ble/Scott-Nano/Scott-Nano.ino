/*
  Button LED

  This example creates a BLE peripheral with service that contains a
  characteristic to control a p5 sketch

  The circuit:
    Arduino Nano 33 BLE
  - Button connected to pin 2

  https://editor.p5js.org/shfitz/sketches/mS3hL2TIB

  Example form Scott Fitzgerald

*/

#include <ArduinoBLE.h>

const int buttonPin = 2; // set buttonPin to digital pin 2

BLEService buttonService("dc52d0c5-efb0-4d41-a779-98f0422da984"); // create service

// create button characteristic and allow remote device to get notifications
BLEByteCharacteristic buttonCharacteristic("eac014e7-91ff-47f1-95d6-966433f5a181", BLERead | BLENotify);

void setup() {
  // usee this if you require debugging
  Serial.begin(9600);
  // while (!Serial); // wait until the serial port is open for debugging

  pinMode(buttonPin, INPUT); // use button pin as an input

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set the local name peripheral advertises
  BLE.setLocalName("Button");
  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(buttonService);

  // add the characteristic to the service
  buttonService.addCharacteristic(buttonCharacteristic);

  // add the service
  BLE.addService(buttonService);

  // initialize a value to send
  buttonCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // poll for BLE events
  BLE.poll();

  // read the current button pin state
  int buttonValue = digitalRead(buttonPin);

  // has the value changed since the last read
  boolean buttonChanged = false;

  if (buttonCharacteristic.value() != buttonValue){
    buttonChanged = true;
  }

  if (buttonChanged) {
    // button state changed, update characteristic
    buttonCharacteristic.writeValue(buttonValue);
    Serial.println(buttonValue);
  }
}
