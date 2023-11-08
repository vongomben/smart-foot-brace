/*
  ArduinoBLE IMU Angles example

  This example creates a BLE peripheral with service that contains a
  characteristic to represents the reading of the potentiometer.

  Arduino Nano 33 Iot

  This example code is in the public domain.

  works with this with  yaw pitch and roll
  https://editor.p5js.org/vongomben/sketches/-2xj3pGf7

*/

#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h> // change to Arduino_LSM6DS3.h for Nano 33 IoT or Uno WiFi Rev 2
#include <Wire.h>

float accelX,            accelY,             accelZ,            // units m/s/s i.e. accelZ if often 9.8 (gravity)
      gyroX,             gyroY,              gyroZ,             // units dps (degrees per second)
      gyroDriftX,        gyroDriftY,         gyroDriftZ,        // units dps
      gyroRoll,          gyroPitch,          gyroYaw,           // units degrees (expect major drift)
      gyroCorrectedRoll, gyroCorrectedPitch, gyroCorrectedYaw,  // units degrees (expect minor drift)
      accRoll,           accPitch,           accYaw,            // units degrees (roll and pitch noisy, yaw not possible)
      complementaryRoll, complementaryPitch, complementaryYaw;  // units degrees (excellent roll, pitch, yaw minor drift)

long lastTime;
long lastInterval;

float angles[3];
int sole[8];

BLEService sensorService("19b10010-e8f2-537e-4f6c-d104768a1214"); // create service

BLECharacteristic AngleCharacteristic ("19b10013-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify, sizeof(float) * 3);
BLECharacteristic SoleCharacteristic ("908e0da6-ebd5-465a-b5db-bf398bc98d9f", BLERead | BLENotify, sizeof(int) * 8);

void setup() {
  Serial.begin(9600);
  //while (!Serial);

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
  BLE.setLocalName("IMU");

  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(sensorService);

  // add the characteristics to the service

  sensorService.addCharacteristic(AngleCharacteristic);

  // add the service
  BLE.addService(sensorService);

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
    //printCalculations();

  }
  angles[0] = complementaryRoll;
  angles[1] = complementaryPitch;
  angles[2] = complementaryYaw;

  sole[1] = random(100);
  sole[2] = random(100);
  sole[3] = random(100);
  sole[4] = random(100);
  sole[5] = random(100);
  sole[6] = random(100);
  sole[7] = random(100);
  sole[8] = random(100);


  AngleCharacteristic.writeValue(angles, sizeof(angles));
  SoleCharacteristic.writeValue(sole, sizeof(sole));


  for (int i = 0; i < sizeof(angles) / sizeof(float); i++) {
    Serial.print(angles[i]);
  }

  Serial.println();
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
