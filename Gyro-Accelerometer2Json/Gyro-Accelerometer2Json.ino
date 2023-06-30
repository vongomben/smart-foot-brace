/*
  Arduino LSM6DS3 - Simple Accelerometer

  This example reads the acceleration values from the LSM6DS3
  sensor and continuously prints them to the Serial Monitor
  or Serial Plotter.

  The circuit:
  - Arduino Uno WiFi Rev 2 or Arduino Nano 33 IoT

  created 10 Jul 2019
  by Riccardo Rizzo

  This example code is in the public domain.
*/

#include <Arduino_LSM6DS3.h>
#include <Arduino_JSON.h>


const int grf8 = A1;
const int grf7 = A0;
const int grf6 = A5;
const int grf5 = A6;
const int grf4 = A7;
const int grf3 = A2;
const int grf2 = A3;
const int grf1 = A4;

// variable for storing the potentiometer value
int gfrValue8 = 0;
int gfrValue7 = 0;
int gfrValue6 = 0;
int gfrValue5 = 0;
int gfrValue4 = 0;
int gfrValue3 = 0;
int gfrValue2 = 0;
int gfrValue1 = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    while (1)
      ;
  }


  IMU.accelerationSampleRate();
  IMU.gyroscopeSampleRate();
}

void loop() {
  float x, y, z;
  float Gx, Gy, Gz;

  JSONVar myObject;

  myObject["vers"] = 1;

  // Reading potentiometer value
  gfrValue8 = analogRead(grf8);
  gfrValue7 = analogRead(grf7);
  /*gfrValue6 = analogRead(grf6);
  gfrValue5 = analogRead(grf5);
  gfrValue4 = analogRead(grf4);
  gfrValue3 = analogRead(grf3);
  gfrValue2 = analogRead(grf2);
  gfrValue1 = analogRead(grf1);*/

  //myObject["gfrValue8"] = gfrValue8;
  //myObject["gfrValue7"] = gfrValue7;
  /*myObject["gfrValue6"] = gfrValue6;
  myObject["gfrValue5"] = gfrValue5;
  myObject["gfrValue4"] = gfrValue4;
  myObject["gfrValue3"] = gfrValue3;
  myObject["gfrValue2"] = gfrValue2;
  myObject["gfrValue1"] = gfrValue1;
*/

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    // Serial.print(x);
    // Serial.print('\t');
    // Serial.print(y);
    // Serial.print('\t');
    // Serial.println(z);


    myObject["x"] = x;
    myObject["y"] = y;
    myObject["z"] = z;
  }

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(Gx, Gy, Gz);
    myObject["Gx"] = Gx;
    myObject["Gy"] = Gy;
    myObject["Gz"] = Gz;
  }

  String jsonString = JSON.stringify(myObject);

  Serial.println(jsonString);

  delay(10);
}
