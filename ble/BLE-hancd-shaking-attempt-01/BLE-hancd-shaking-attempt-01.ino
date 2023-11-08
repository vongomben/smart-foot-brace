#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

/*

 BLESerial was written by Ian Archbell of oddWires. It is based on the BLE implementation
 originally created by Neil Kolban and included in the Espressif esp32 distribution.
 This library makes it simple to send and received data that would normally go to or be sent by
 the serial monitor. The interface is very similar so most usage is identical.

 To use:

  1. Create an instance of BLESerial 
  2. Make sure you call begin()   

 In this example we use BLESerial to use in a similair way as the "graph" example in the IDE. 
 You could use this P5 sketch to plot the data from the Arduino (if using chrome)
 https://editor.p5js.org/lemio/sketches/qKGGxBG4C
 Or use the Adafruit "Bluefruit Connect" App. 

*/

#include "BLESerial.h"
BLESerial bleSerial;

// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

LSM9DS1 imu;

#include <Arduino_JSON.h>

float angles[3];



////////////////////////////
// Sketch Output Settings //
////////////////////////////
#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 250              // 250 ms between prints
static unsigned long lastPrint = 0;  // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract
// a declination to get a more accurate heading. Calculate
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION 3.11  // Declination (degrees) in Turin, IT.


//Function definitions
void printGyro();
void printAccel();
void printMag();
void printAttitude(float ax, float ay, float az, float mx, float my, float mz);


bool waitForAck = false;
unsigned long ackTimeout = 200; // Tempo massimo di attesa dell'ack in millisecondi
unsigned long lastAckTime;

// Invia i dati via BLE
void sendData(String data) {
  bleSerial.println(data);
  waitForAck = true;
  lastAckTime = millis();
}

void setup() {
  Serial.begin(115200);
  bleSerial.begin("Smart-Sole-01");

  Wire.begin();

  if (imu.begin() == false)  // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will "
                   "work for an out of the box LSM9DS1 "
                   "Breakout, but may need to be modified "
                   "if the board jumpers are.");
    while (1)
      ;
  }
}

void loop() {

  JSONVar myObject;

  //myObject["vers"] = 1;

  if (imu.gyroAvailable()) {
    imu.readGyro();
  }
  if (imu.accelAvailable()) {
    imu.readAccel();
  }
  if (imu.magAvailable()) {
    imu.readMag();
  }

  // getting roll pitch and heading inside of the loop

  float roll = atan2(imu.ay, imu.az);
  float pitch = atan2(-imu.ax, sqrt(imu.ay * imu.ay + imu.az * imu.az));

  float heading;
  if (imu.my == 0)
    heading = (imu.mx < 0) ? PI : 0;
  else
    heading = atan2(imu.mx, imu.my);

  heading -= DECLINATION * PI / 180;

  if (heading > PI) heading -= (2 * PI);
  else if (heading < -PI) heading += (2 * PI);

  // Convert everything from radians to degrees:
  heading *= 180.0 / PI;
  pitch *= 180.0 / PI;
  roll *= 180.0 / PI;

  myObject["ax"] = roll;
  myObject["ay"] = pitch;
  myObject["az"] = heading;


  // JSON.stringify(myVar) can be used to convert the JSONVar to a String
  String jsonString = JSON.stringify(myObject);

    //Serial.println(jsonString);

  String dataset = String(roll) + "|" + String(pitch) + "|" + String(heading);
  //Serial.println(dataset);


  if (bleSerial.connected()) {

    bleSerial.println(jsonString);
    Serial.println(jsonString);

     delay(50);


  }

}
