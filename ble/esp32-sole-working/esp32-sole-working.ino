
/*

Working SMart sole ble communication sketch to p5js
thanks to the help of mpiccin!

vongomben, nov 8, 23 Turin IT



*/



#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "BLESerial.h"
BLESerial bleSerial;

// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

LSM9DS1 imu;

float angles[3];

// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int grf8 = 39;
const int grf7 = 34;  //36
const int grf6 = 25;
const int grf5 = 26;
const int grf4 = 14;
const int grf3 = 35;
const int grf2 = 32;
const int grf1 = 33;


// variable for storing the potentiometer value
int gfrValue8 = 0;
int gfrValue7 = 0;
int gfrValue6 = 0;
int gfrValue5 = 0;
int gfrValue4 = 0;
int gfrValue3 = 0;
int gfrValue2 = 0;
int gfrValue1 = 0;


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

  gfrValue8 = analogRead(grf8);
  gfrValue7 = analogRead(grf7);
  gfrValue6 = analogRead(grf6);
  gfrValue5 = analogRead(grf5);
  gfrValue4 = analogRead(grf4);
  gfrValue3 = analogRead(grf3);
  gfrValue2 = analogRead(grf2);
  gfrValue1 = analogRead(grf1);

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

  angles[0] = roll;
  angles[1] = pitch;
  angles[2] = heading;

  float ax = roll;
  float ay = pitch;
  float az = heading;
  uint16_t values[] = { gfrValue8, gfrValue7, gfrValue6, gfrValue5, gfrValue4, gfrValue3, gfrValue2, gfrValue1 };

  unsigned int dataLength = 3 * sizeof(float) + 8 * sizeof(uint16_t);
  byte dataBytes[dataLength];

  // Copia dei valori float nell'array di byte
  memcpy(dataBytes, &ax, sizeof(ax));
  memcpy(dataBytes + sizeof(ax), &ay, sizeof(ay));
  memcpy(dataBytes + sizeof(ax) + sizeof(ay), &az, sizeof(az));

  // Copia dei valori uint16_t nell'array di byte
  for (int i = 0; i < 8; i++) {
    memcpy(dataBytes + 3 * sizeof(float) + i * sizeof(uint16_t), &values[i], sizeof(uint16_t));
  }

  //String dataset1 = String(roll) + "|" + String(pitch) + "|" + String(heading) + "|" + String(gfrValue8) + "|" + String(gfrValue7) + "|" + String(gfrValue6) + "|" + String(gfrValue5) + "|" + String(gfrValue4) + "|" + String(gfrValue3) + "|" + String(gfrValue2) + "|" + String(gfrValue1) + "?";


  //If we're connected
  if (bleSerial.connected()) {
    //Send the analog value
    //for (int i = 0; i < 3; i++) {  // Assuming angles has 3 elements
    bleSerial.print(roll);
    Serial.print(roll);

    bleSerial.print("|");
    Serial.print("|");

    bleSerial.print(pitch);
    Serial.print(pitch);

    bleSerial.print("|");
    Serial.print("|");

    bleSerial.print(heading);
    Serial.print(heading);

    bleSerial.print("|");
    Serial.print("|");

    bleSerial.print(gfrValue8);
    Serial.print(gfrValue8);

    bleSerial.print("|");
    Serial.print("|");
    
    bleSerial.print(gfrValue7);
    Serial.print(gfrValue7);

    bleSerial.print("|");
    Serial.print("|");
    
    bleSerial.print(gfrValue6);
    Serial.print(gfrValue6);

    bleSerial.print("|");
    Serial.print("|");
    
    bleSerial.print(gfrValue5);
    Serial.print(gfrValue5);

    bleSerial.print("|");
    Serial.print("|");

    bleSerial.print(gfrValue4);
    Serial.print(gfrValue4);

    bleSerial.print("|");
    Serial.print("|");

    bleSerial.print(gfrValue3);
    Serial.print(gfrValue3);
    
    bleSerial.print("|");
    Serial.print("|");

    bleSerial.print(gfrValue2);
    Serial.print(gfrValue2);

    bleSerial.print("|");
    Serial.print("|");
    
    bleSerial.println(gfrValue1);
    Serial.println(gfrValue1);
    //bleSerial.write(dataBytes, dataLength);
    //bleSerial.print(dataset2);
    //bleSerial.flush();
    //Serial.println(dataset1);
    delay(10);
    /*bleSerial.print(pitch);
    Serial.print(pitch);
    bleSerial.print(heading);
    Serial.print(heading);
*/

    //bleSerial.println();
    //Serial.println();
  }

  //  bleSerial.println(angles);
  //   Serial.println(jsonString);
  //Wait for 0.1 second
}