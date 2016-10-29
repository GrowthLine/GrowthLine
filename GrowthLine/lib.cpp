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

Sensor::~Sensor() {}                                                        // Needed to destruct the Sensor objects

/* ~~~~~~~~~~~~~~~~~~~~~~~ Light Sensor Class ~~~~~~~~~~~~~~~~~~~~~~~ */
LightSensor::LightSensor(Reading *r) {
  reading = r;
}

LightSensor::~LightSensor() {}                                              // Needed to destruct the Sensor base object

void LightSensor::read() {
  sensors_event_t event;
  luxSensor.getEvent(&event);
  Serial.print("Lux inside the read method"); Serial.println(event.light);
  reading->lux = event.light;
}

void LightSensor::setUp() {
  luxSensor.enableAutoRange(true);                                           // switches automatically between the different gains
  luxSensor.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);               // slowest but most accurate
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ Temperature-Humidity Sensor ~~~~~~~~~~~~~~~~~~~~~~~ */
//TempHumid::TempHumid(Reading *r, int p ) {
//  reading = r;
//  pin = p;
//}
//
//TempHumid::~TempHumid() {}                                                    // Needed to destruct the sensor base object
//
//void TempHumid::read() {
//  int sensorStatus = DHT.read22(pin);
//  reading->humidity = DHT.humidity;
//  reading->
//}













