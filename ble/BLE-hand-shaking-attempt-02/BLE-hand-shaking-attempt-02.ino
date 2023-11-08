#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "BLESerial.h"
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>
#include <Arduino_JSON.h>

BLESerial bleSerial;
LSM9DS1 imu;
JSONVar myObject;
#define DECLINATION 3.11 // Declination in Turin, IT
bool waitForAck = false;
unsigned long ackTimeout = 200; // Timeout for ack in milliseconds
unsigned long lastAckTime;

void setup() {
  Serial.begin(115200);
  bleSerial.begin("Smart-Sole-01");
  Wire.begin();
  
  if (!imu.begin()) {
    Serial.println("Failed to communicate with LSM9DS1.");
    while (1);
  }
}

void loop() {
  if (imu.gyroAvailable()) {
    imu.readGyro();
  }
  if (imu.accelAvailable()) {
    imu.readAccel();
  }
  if (imu.magAvailable()) {
    imu.readMag();
  }

  // Calculate orientation
  float roll = atan2(imu.ay, imu.az);
  float pitch = atan2(-imu.ax, sqrt(imu.ay * imu.ay + imu.az * imu.az));
  float heading = calculateHeading(imu.mx, imu.my);

  myObject["ax"] = roll * 180.0 / PI;
  myObject["ay"] = pitch * 180.0 / PI;
  myObject["az"] = heading;

  String jsonString = JSON.stringify(myObject);

  if (bleSerial.connected() && !waitForAck) {
    bleSerial.println(jsonString);
    Serial.println(jsonString);
    waitForAck = true;
    lastAckTime = millis();
  }

  if (waitForAck && (millis() - lastAckTime) > ackTimeout) {
    waitForAck = false; // Reset and handle resend or error
    Serial.println("Ack not received, resending or handling error.");
  }

  // Check for incoming ack
  if (bleSerial.available()) {
    String incomingData = bleSerial.readStringUntil('\n');
    if (incomingData == "ack") {
      waitForAck = false; // Ack received, ready for next data send
    }
  }

  delay(100); // A delay to prevent flooding; adjust as needed
}

float calculateHeading(float mx, float my) {
  float heading = atan2(mx, my);
  heading -= DECLINATION * PI / 180;
  if (heading > PI) heading -= (2 * PI);
  else if (heading < -PI) heading += (2 * PI);
  heading *= 180.0 / PI;
  return heading;
}
