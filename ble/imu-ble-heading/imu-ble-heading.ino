/*
  ArduinoBLE Potentiometer and Button

  This example creates a BLE peripheral with service that contains a
  characteristic to represents the reading of the potentiometer.

  The circuit:
  - Arduino MKR WiFi 1010 or Arduino Uno WiFi Rev2 board
  - Potentiometer connected to pin A2

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h> // change to Arduino_LSM6DS3.h for Nano 33 IoT or Uno WiFi Rev 2
#include <Wire.h>


const int sensorPin = A2; // set buttonPin to digital pin 4
const int buttonPin = 3; // set buttonPin to digital pin 4

float accelX,            accelY,             accelZ,            // units m/s/s i.e. accelZ if often 9.8 (gravity)
      gyroX,             gyroY,              gyroZ,             // units dps (degrees per second)
      gyroDriftX,        gyroDriftY,         gyroDriftZ,        // units dps
      gyroRoll,          gyroPitch,          gyroYaw,           // units degrees (expect major drift)
      gyroCorrectedRoll, gyroCorrectedPitch, gyroCorrectedYaw,  // units degrees (expect minor drift)
      accRoll,           accPitch,           accYaw,            // units degrees (roll and pitch noisy, yaw not possible)
      complementaryRoll, complementaryPitch, complementaryYaw;  // units degrees (excellent roll, pitch, yaw minor drift)

long lastTime;
long lastInterval;

BLEService sensorService("19B10010-E8F2-537E-4F6C-D104768A1214"); // create service

// create button characteristic and allow remote device to read and write
//BLEIntCharacteristic sensorCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
//BLEBoolCharacteristic buttonCharacteristic("19B10013-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
// BLECharacteristic accelerationCharacteristic ("2A5A20B9-0001-4B9C-9C69-4975713E0FF2", BLENotify, sizeof(float) * 3);
//BLEIntCharacteristic accelerationCharacteristic ("2A5A20B9-0001-4B9C-9C69-4975713E0FF2", BLERead | BLENotify);
//BLEIntCharacteristic gyroscopeCharacteristic    ("2A5A20B9-0002-4B9C-9C69-4975713E0FF2", BLERead | BLENotify);
BLECharacteristic RollCharacteristic ("7b193e68-e545-4e5b-9693-c4f9535150ee", BLERead | BLENotify, sizeof(float));
BLECharacteristic PitchCharacteristic ("a057c9f5-148c-4963-9d07-1b098681f04d", BLERead | BLENotify, sizeof(float));
BLECharacteristic YawCharacteristic ("908e0da6-ebd5-465a-b5db-bf398bc98d9f", BLERead | BLENotify, sizeof(float));
//BLECharacteristic PCharacteristic ("19B10013-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, sizeof(float) * 3);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(sensorPin, INPUT); // use button pin as an input
  pinMode(buttonPin, INPUT); // use button pin as an input

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  calibrateIMU(250, 250);

  lastTime = micros();

  if (!BLE.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set the local name peripheral advertises
  BLE.setLocalName("Sensor Button");

  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(sensorService);

  // add the characteristics to the service
  //sensorService.addCharacteristic(sensorCharacteristic);
  //sensorService.addCharacteristic(buttonCharacteristic);
  //sensorService.addCharacteristic(accelerationCharacteristic);
  //sensorService.addCharacteristic(gyroscopeCharacteristic);

  sensorService.addCharacteristic(RollCharacteristic);
  sensorService.addCharacteristic(PitchCharacteristic);
  sensorService.addCharacteristic(YawCharacteristic);


  // add the service
  BLE.addService(sensorService);


  //set init value
  //sensorCharacteristic.writeValue(0);
  //buttonCharacteristic.writeValue(0);


  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}


/*
  the gyro's x,y,z values drift by a steady amount. if we measure this when arduino is still
  we can correct the drift when doing real measurements later
*/
void calibrateIMU(int delayMillis, int calibrationMillis) {

  int calibrationCount = 0;

  delay(delayMillis); // to avoid shakes after pressing reset button

  float sumX, sumY, sumZ;
  int startTime = millis();
  while (millis() < startTime + calibrationMillis) {
    if (readIMU()) {
      // in an ideal world gyroX/Y/Z == 0, anything higher or lower represents drift
      sumX += gyroX;
      sumY += gyroY;
      sumZ += gyroZ;

      calibrationCount++;
    }
  }

  if (calibrationCount == 0) {
    Serial.println("Failed to calibrate");
  }

  gyroDriftX = sumX / calibrationCount;
  gyroDriftY = sumY / calibrationCount;
  gyroDriftZ = sumZ / calibrationCount;

}

/**
   Read accel and gyro data.
   returns true if value is 'new' and false if IMU is returning old cached data
*/
bool readIMU() {
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable() ) {
    IMU.readAcceleration(accelX, accelY, accelZ);
    IMU.readGyroscope(gyroX, gyroY, gyroZ);
    return true;
  }
  return false;
}


void loop() {
  // poll for BLE events
  BLE.poll();

  if (readIMU()) {
    long currentTime = micros();
    lastInterval = currentTime - lastTime; // expecting this to be ~104Hz +- 4%
    lastTime = currentTime;

    doCalculations();
    printCalculations();

  }

  /* int sensorValue = analogRead(sensorPin);
    //  Serial.println(sensorValue / 4);
    sensorCharacteristic.writeValue(sensorValue / 4);
    bool buttonValue = digitalRead(buttonPin);
    buttonCharacteristic.writeValue(buttonValue);
*/
RollCharacteristic.writeValue(static_cast<float&>(complementaryRoll));
PitchCharacteristic.writeValue(static_cast<float&>(complementaryPitch));
YawCharacteristic.writeValue(static_cast<float&>(complementaryYaw));
  
/*
  float tempRoll = complementaryRoll;
  float tempPitch = complementaryPitch;
  float tempYaw = complementaryYaw;

  RollCharacteristic.writeValue(tempRoll);
  PitchCharacteristic.writeValue(tempPitch);
  YawCharacteristic.writeValue(tempYaw);

  /*
    RollCharacteristic.writeValue(accRoll);
    PitchCharacteristic.writeValue(accPitch);
    YawCharacteristic.writeValue(accYaw);
  */

}


/**
   I'm expecting, over time, the Arduino_LSM6DS3.h will add functions to do most of this,
   but as of 1.0.0 this was missing.
*/
void doCalculations() {
  accRoll = atan2(accelY, accelZ) * 180 / M_PI;
  accPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180 / M_PI;

  float lastFrequency = (float) 1000000.0 / lastInterval;
  gyroRoll = gyroRoll + (gyroX / lastFrequency);
  gyroPitch = gyroPitch + (gyroY / lastFrequency);
  gyroYaw = gyroYaw + (gyroZ / lastFrequency);

  gyroCorrectedRoll = gyroCorrectedRoll + ((gyroX - gyroDriftX) / lastFrequency);
  gyroCorrectedPitch = gyroCorrectedPitch + ((gyroY - gyroDriftY) / lastFrequency);
  gyroCorrectedYaw = gyroCorrectedYaw + ((gyroZ - gyroDriftZ) / lastFrequency);

  complementaryRoll = complementaryRoll + ((gyroX - gyroDriftX) / lastFrequency);
  complementaryPitch = complementaryPitch + ((gyroY - gyroDriftY) / lastFrequency);
  complementaryYaw = complementaryYaw + ((gyroZ - gyroDriftZ) / lastFrequency);

  complementaryRoll = 0.98 * complementaryRoll + 0.02 * accRoll;
  complementaryPitch = 0.98 * complementaryPitch + 0.02 * accPitch;
}

/**
   This comma separated format is best 'viewed' using 'serial plotter' or processing.org client (see ./processing/RollPitchYaw3d.pde example)
*/
void printCalculations() {
  /* Serial.print(gyroRoll);
    Serial.print(',');
    Serial.print(gyroPitch);
    Serial.print(',');
    Serial.print(gyroYaw);
    Serial.print(',');
    Serial.print(gyroCorrectedRoll);
    Serial.print(',');
    Serial.print(gyroCorrectedPitch);
    Serial.print(',');
    Serial.print(gyroCorrectedYaw);
    Serial.print(',');
    Serial.print(accRoll);
    Serial.print(',');
    Serial.print(accPitch);
    Serial.print(',');
    Serial.print(accYaw);
    Serial.print(',');
  */
  Serial.print(complementaryRoll);
  Serial.print(',');
  Serial.print(complementaryPitch);
  Serial.print(',');
  Serial.print((int)complementaryYaw);
  Serial.println("");

}
