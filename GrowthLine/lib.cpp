#include "lib.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~ Reading Class ~~~~~~~~~~~~~~~~~~~~~~~ */
Reading::Reading( float l, float aT, float h, float ph, float gT, float m ) {
  lux = l;
  airTemperature = aT;
  humidity = h;
  pH = ph;
  groundTemperature = gT;
  moisture = m;
}

Reading::Reading() {
  Reading( 0, 0, 0, 0, 0, 0);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ Sensor Abstract Class ~~~~~~~~~~~~~~~~~~~~~~~ */

Sensor::~Sensor() {}                                                        // Needed to destruct the Sensor objects

/* ~~~~~~~~~~~~~~~~~~~~~~~ Light Sensor Class ~~~~~~~~~~~~~~~~~~~~~~~ */
LightSensor::LightSensor(Reading *r) {
  reading = r;
}

LightSensor::~LightSensor() {}                                              // Needed to destruct the Sensor base object

void LightSensor::read() {
  sensors_event_t event;
  luxSensor.getEvent(&event);
  reading->lux = event.light;
}

void LightSensor::setUp() {
  luxSensor.enableAutoRange(true);                                           // switches automatically between the different gains
  luxSensor.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);               // slowest but most accurate
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ Temperature-Humidity Sensor ~~~~~~~~~~~~~~~~~~~~~~~ */
TempHumid::TempHumid(Reading *r, int p ) {
  reading = r;
  pin = p;
}

TempHumid::~TempHumid() {}                                                    // Needed to destruct the sensor base object

void TempHumid::read() {
  int sensorStatus = DHT.read22(pin);
  reading->humidity = DHT.humidity;
  reading->airTemperature = DHT.temperature;
}

void setUp() {}

/* ~~~~~~~~~~~~~~~~~~~~~~~ pH Sensor ~~~~~~~~~~~~~~~~~~~~~~~ */

pH::pH(Reading *r, int re, int t) {
  reading = r;
  rx = re;
  tx = t;
  serial = new SoftwareSerial(rx, tx);
  serial->begin(9600);
}

pH::~pH() {}

void pH::read() {
  serial->print("R\r");
  String sensorString = "";
  while( serial->available() > 0 ) {
    char inchar = (char)serial->read();
    if(inchar == '\r')
      break;  
    sensorString += inchar;   
  }
  reading->pH = sensorString.toFloat();
}

void pH::setUp() {}

/* ~~~~~~~~~~~~~~~~~~~~~~~ Temperatue / Moisture Class ~~~~~~~~~~~~~~~~~~~~~~~ */  // Not done Yet
TempMoist::TempMoist(Reading *r, int d, int c) {
  reading = r;
  dataPin = d;
  clockPin = c;
}

TempMoist::~TempMoist() {}

void TempMoist::read() {
  
}

void TempMoist::setUp() {
  
}










