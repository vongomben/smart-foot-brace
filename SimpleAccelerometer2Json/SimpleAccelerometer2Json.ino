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

void setup() {
  Serial.begin(9600);
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
