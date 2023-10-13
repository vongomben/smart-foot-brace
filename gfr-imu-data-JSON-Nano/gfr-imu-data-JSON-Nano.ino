// SNZ-01 Smart Sole example code with basic MPU6050 IMU
//
// exposing the 8 grf points and MPU6050 IMU as a JSON Object
//
// 16/19 June 2023, Turin, Davide Gomba
//
// Arduino Nano 33 IoT Example

#include <Arduino_LSM6DS3.h>

#include <Arduino_JSON.h>


// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
// const int grf8 = 39;
// const int grf7 = 36;
// const int grf6 = 33;
// const int grf5 = 25;
// const int grf4 = 26;
// const int grf3 = 34;
// const int grf2 = 35;
// const int grf1 = 32;

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
  delay(1000);


}

void loop() {

  JSONVar myObject;

  myObject["vers"] = 1;

  // Reading potentiometer value
  gfrValue8 = analogRead(grf8);
  gfrValue7 = analogRead(grf7);
  gfrValue6 = analogRead(grf6);
  gfrValue5 = analogRead(grf5);
  gfrValue4 = analogRead(grf4);
  gfrValue3 = analogRead(grf3);
  gfrValue2 = analogRead(grf2);
  gfrValue1 = analogRead(grf1);

  myObject["gfrValue8"] = gfrValue8;
  myObject["gfrValue7"] = gfrValue7;
  myObject["gfrValue6"] = gfrValue6;
  
  myObject["gfrValue5"] = gfrValue5;
  myObject["gfrValue4"] = gfrValue4;
  myObject["gfrValue3"] = gfrValue3;
  myObject["gfrValue2"] = gfrValue2;
  myObject["gfrValue1"] = gfrValue1;


  // JSON.stringify(myVar) can be used to convert the JSONVar to a String
  String jsonString = JSON.stringify(myObject);

  Serial.println(jsonString);

  //  Serial.println();

  delay(20);
}