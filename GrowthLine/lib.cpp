#include "lib.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~ General Functions ~~~~~~~~~~~~~~~~~~~~~~~ */
/* Finds the quadrant touched by the user on the screen */
 uint8_t getQuadrantFromPoint(TS_Point *p) {
  if ( p->x > TS_MAXX / 2 && p->y < TS_MAXY / 2) {
    return BTN_NW;
  }
  if ( p->x < TS_MAXX / 2 && p->y < TS_MAXY / 2) {
    return BTN_SW;
  }
  if ( p->x < TS_MAXX / 2 && p->y > TS_MAXY / 2) {
    return BTN_SE;
  }
  if ( p->x > TS_MAXX / 2 && p->y > TS_MAXY / 2) {
    return BTN_NE;
  }
  return BTN_NONE;
}

/* converts Celsious to Fahrenheit */
float cToF(float c) {
  return c * 1.8 + 32;
}

/* analyzse the readings list and see if the readings are stable */
bool stableReadings(QueueList<Reading> *readings) {
  const float tolerance = WARMUP_TOLERANCE;
  Statistic phStats;
  phStats.clear();
  Statistic grndTempStats;
  grndTempStats.clear();
  Statistic moistureStats;
  moistureStats.clear();
  Statistic airTempStats;
  airTempStats.clear();
  Statistic humidityStats;
  humidityStats.clear();

  for( int i = 0; i < readings->count(); i++) {
    phStats.add(readings->peek().pH);
    grndTempStats.add(readings->peek().groundTemperature);
    moistureStats.add(readings->peek().moisture);
    airTempStats.add(readings->peek().airTemperature);
    humidityStats.add(readings->peek().humidity);
  }
  if(phStats.unbiased_stdev() >= phStats.average() * tolerance)
    return false;
  if(grndTempStats.unbiased_stdev() >= grndTempStats.average() * tolerance)
    return false;
  if(moistureStats.unbiased_stdev() >= moistureStats.average() * tolerance)
    return false;
  if(airTempStats.unbiased_stdev() >= airTempStats.average() * tolerance)
    return false;
  if(humidityStats.unbiased_stdev() >= humidityStats.average() * tolerance)
    return false;
  return true;
}

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

String Reading::toString() {
  String line = "";
  line = line + groundTemperature + "," + moisture + "," + pH + "," + airTemperature + "," + humidity + "," + lux;
  return line;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ Sensor Abstract Class ~~~~~~~~~~~~~~~~~~~~~~~ */

Sensor::~Sensor() {}                                                        // Needed to destruct the Sensor objects

/* ~~~~~~~~~~~~~~~~~~~~~~~ Light Sensor Class ~~~~~~~~~~~~~~~~~~~~~~~ */
LightSensor::LightSensor() {
  ID = LUX_SENSOR_ID;
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

void LightSensor::setReading(Reading *r) {
  reading = r;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ Temperature-Humidity Sensor ~~~~~~~~~~~~~~~~~~~~~~~ */
TempHumid::TempHumid(uint8_t p ) {
  ID = TEMP_HUMID_SENSOR_ID;
  pin = p;
  DHT = new DHT_Unified(p, DHT22);
}

TempHumid::~TempHumid() {}                                                    // Needed to destruct the sensor base object

void TempHumid::read() {
  sensors_event_t event;
  DHT->temperature().getEvent(&event);
  reading->airTemperature = event.temperature;
  DHT->humidity().getEvent(&event);
  reading->humidity = event.relative_humidity;
}

void TempHumid::setUp() {}
void TempHumid::setReading(Reading *r) {
  reading = r;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ pH Sensor ~~~~~~~~~~~~~~~~~~~~~~~ */

pH::pH(uint8_t re, uint8_t t) {
  ID = PH_SENSOR_ID;
  rx = re;
  tx = t;
  serial = new SoftwareSerial(rx, tx);
  serial->begin(9600);
  serial->print("RESPONSE,0\r");        //Disables status responses
}

pH::~pH() {}

void pH::read() {
  serial->print("R\r");
  String sensorString = "";
  while ( serial->available() > 0 ) {
    char inchar = (char)serial->read();
    if (inchar == '\r')
      break;
    sensorString += inchar;
  }
  while( serial->available() > 0)     // Clears the buffer
    serial->read();
  
  reading->pH = sensorString.toFloat();
}

void pH::setUp() {}

void pH::setReading(Reading *r) {
  reading = r;

}

void pH::calibrate() {
  serial->print("Cal,mid,7.00\r");
  while( serial->available() > 0)     // Clears the buffer
    serial->read();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ Temperatue / Moisture Class ~~~~~~~~~~~~~~~~~~~~~~~ */ 
TempMoist::TempMoist(uint8_t d, uint8_t c) {
  ID = TEMP_MOIST_SENSOR_ID;
  dataPin = d;
  clockPin = c;
  sht1x = new SHT1x(dataPin, clockPin);
}

TempMoist::~TempMoist() {}

void TempMoist::read() {
  reading->groundTemperature = sht1x->readTemperatureC();
  reading->moisture = sht1x->readHumidity();
}

void TempMoist::setUp() {}

void TempMoist::setReading(Reading *r) {
  reading = r;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~  Sensors Class ~~~~~~~~~~~~~~~~~~~~~~~ */
Sensors::Sensors() {}

/* Adds a sensor to the Sensors object */
void Sensors::addSensor(Sensor *s) {
  sensors.push(s);
}

/* Calls the setup function of each sensor in the Sensors object */
void Sensors::setupSensors() {
  for (uint8_t i = 0; i < sensors.count(); i++ ) {
    Sensor *s = sensors.pop();
    s->setUp();
    sensors.push(s);
  }
}

/* calls the read function of each sensor and returns a reading
    object with all the reads
*/
Reading Sensors::getReading() {
  Reading reading;
  for ( uint8_t i = 0; i < sensors.count(); i++) {
    Sensor *s = sensors.pop();
    s->setReading(&reading);
    sensors.push(s);
  }
  for ( uint8_t i = 0; i < sensors.count(); i++) {
    Sensor *s = sensors.pop();
    s->read();
    sensors.push(s);
  }
  return reading;
}

/* returns a pointer to the sensor who's ID matches the given ID */
Sensor* Sensors::getSensor(uint8_t id) {
  Sensor *sensor;
  for ( uint8_t i = 0; i < sensors.count(); i++ ) {
    sensor = sensors.pop();
    if ( sensor->ID == id ) {
      sensors.push(sensor);
      return sensor;
    }
    else
      sensors.push(sensor);
  }
  return NULL;
}









