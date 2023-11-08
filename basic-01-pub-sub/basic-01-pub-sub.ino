// SNZ-01 Smart Sole example code with basic SFE_LSM9DS1 IMU
//
// exposing the 8 grf points and LSM9DS1 IMU as a JSON Object
//
// 17 October 2023, Turin, Davide Gomba
//
// ESP32 dev module IoT Example

// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.

#include <WiFi.h>
const char* mqtt_server = "broker.hivemq.com";  //"192.168.0.15"//
#define DEV_NAME "mqttdevice"
#define MQTT_USER ""
#define MQTT_PW ""

#include <MQTT.h>

const char* ssid = "salottino";
const char* password = "scherze771";


WiFiClient net;
PubSubClient client(espClient);
unsigned long lastMillis = 0;
long lastMsg = 0;
char msg[50];
int value = 0;

#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

#include <Arduino_JSON.h>


LSM9DS1 imu;

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

//Function definitions
void printGyro();
void printAccel();
void printMag();
void printAttitude(float ax, float ay, float az, float mx, float my, float mz);


void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  while (!client.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
  client.subscribe("/hello");  //SUBSCRIBE TO TOPIC /hello
}
void messageReceived(String& topic, String& payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if (topic == "/hello") {
    if (payload == "open") {
      Serial.println("open");
      //   digitalWrite(LED_BUILTIN, HIGH);
    } else if (payload == "closed") {
      Serial.println("closed");
      // digitalWrite(LED_BUILTIN, LOW);
    }
  }
}




void setup() {
  Serial.begin(115200);
  delay(1000);

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

  WiFi.begin(ssid, pass);
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  //
  // MQTT brokers usually use port 8883 for secure connections.
  client.begin(BROKER_IP, 1883, net);
  client.onMessage(messageReceived);
  connect();
}

void loop() {

  client.loop();
  if (!client.connected()) {
    connect();
  }

  // Update the sensor values whenever new data is available
  if (imu.gyroAvailable()) {
    // To read from the gyroscope,  first call the
    // readGyro() function. When it exits, it'll update the
    // gx, gy, and gz variables with the most current data.
    imu.readGyro();
  }
  if (imu.accelAvailable()) {
    // To read from the accelerometer, first call the
    // readAccel() function. When it exits, it'll update the
    // ax, ay, and az variables with the most current data.
    imu.readAccel();
  }
  if (imu.magAvailable()) {
    // To read from the magnetometer, first call the
    // readMag() function. When it exits, it'll update the
    // mx, my, and mz variables with the most current data.
    imu.readMag();
  }

  JSONVar myObject;

  myObject["vers"] = 2;

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

  myObject["pitch"] = pitch;
  myObject["roll"] = roll;
  myObject["heading"] = heading;


  // JSON.stringify(myVar) can be used to convert the JSONVar to a String
  String jsonString = JSON.stringify(myObject);

  Serial.println(jsonString);


  //  Serial.println();

  delay(20);

  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    Serial.println(jsonString);

    client.publish("/smart-foot-brace", jsonString);  //PUBLISH TO TOPIC /hello MSG world
  }
}