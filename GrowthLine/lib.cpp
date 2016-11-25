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

void getLogs(String fileName, String logs[], unsigned int *logNumber ) {
  File logFile = SD.open(fileName);
  /* Read data already displayed in the log scereen */
  uint8_t logCounter = 0;
  while (logCounter != *logNumber && logFile.available() ) {
    if (logFile.read() == '\n')
      logCounter += 1 ;
  }
  
  if(!logFile.available()) {                             // if the file has been fully read, just exit
    logFile.close();
    return;
  }
    
  uint8_t index = 0;                                  // current array index
  bool use = true;                                    // used to tell whether we save character or not
  uint8_t commaCount = 0;                             // used to tell when we arrive to pH field
  while ( logFile.available() ) {
    if ( index == 5 )                                 // once we have 5 logs, exit
      break;
    char current = logFile.read();
    if ( current == '\n') {                           // we finished reading one record
      index += 1;
      commaCount = 0;
      use = true;
      continue;
    }
    if ( current == '.' && commaCount != 3) {         // truncate fractinoal part, except for pH
      use = false;
      continue;
    }
    else if ( current == ',') {                       // start saving characters again, for next fields
      commaCount += 1;
      use = true;
    }
    if ( use )                                        // concatinate the current character to the current log
      logs[index] += current;
  }
  logFile.close();
  *logNumber += index;
}


//Overwrite "settings.txt" when changes are made
void saveSettings (unsigned int logFileNumber, unsigned int readingNumber, bool saveEnable, bool fahrenheit){
  if (saveEnable){
      SD.remove("settings.txt");
      File settingsFile = SD.open("settings.txt", FILE_WRITE);
      settingsFile.println("TempUnit=" + fahrenheit? "F" : "C" );
      settingsFile.println("LogFile=" + String(logFileNumber));
      settingsFile.println("Reading=" + String(readingNumber));
      settingsFile.close();
      Serial.println("Settings Updated!");
  }
  else{
    Serial.println("Settings Not Updated!");
  }
}

//Save log to SD card
void saveLog(unsigned int logFileNumber, unsigned int *readingNumber, QueueList<Reading> *readings, bool fahrenheit){
  String logFileName = "log" + String(logFileNumber) + ".txt";
  if (SD.exists(logFileName)) {
    File logFile = SD.open(logFileName, FILE_WRITE);
    logFile.println(String(*readingNumber) + "," + readings->peek().toString(fahrenheit));
    logFile.close();
    *readingNumber += 1;
  }
  else {
    Serial.println("Log File Not Found!");
  }
}

//Check if log file exists and create it if it does not.
void checkLogExists(unsigned int logFileNumber){
  String logFileName = "log" + String(logFileNumber) + ".txt";
  String fileHeader = "Reading Number,Soil Temperature,Soil Moisture,Soil pH,Air Temperature,Air Humidity,Lux";
  if (!SD.exists(logFileName)) {
    File logFile = SD.open(logFileName, FILE_WRITE);
    logFile.println(fileHeader);
    logFile.close();
    Serial.println("New log file created " + logFileName);
  }
  else {
    Serial.println("Log File Found!");
  }
}

uint8_t phStatus(float ph) {
  if ( ph < GOOD_LOW_PH && ph > 0 )
    return BAD_LOW;
  else if ( ph >= GOOD_LOW_PH && ph <= GOOD_HIGH_PH)
    return GOOD;
  else if ( ph > GOOD_HIGH_PH && ph < 14 )
    return BAD_HIGH;
  else
    return UNKNOWN;
}
uint8_t groundTempStatus(float t) {
  if ( t < GOOD_LOW_GROUND_TEMPERATURE && t > 0 )
    return BAD_LOW;
  else if ( t >= GOOD_LOW_GROUND_TEMPERATURE && t <= GOOD_HIGH_GROUND_TEMPERATURE)
    return GOOD;
  else if ( t > GOOD_HIGH_GROUND_TEMPERATURE && t < 14 )
    return BAD_HIGH;
  else
    return UNKNOWN;
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

  for ( int i = 0; i < readings->count(); i++) {
    phStats.add(readings->peek().pH);
    grndTempStats.add(readings->peek().groundTemperature);
    moistureStats.add(readings->peek().moisture);
    airTempStats.add(readings->peek().airTemperature);
    humidityStats.add(readings->peek().humidity);
  }
  if (phStats.unbiased_stdev() >= phStats.average() * tolerance)
    return false;
  if (grndTempStats.unbiased_stdev() >= grndTempStats.average() * tolerance)
    return false;
  if (moistureStats.unbiased_stdev() >= moistureStats.average() * tolerance)
    return false;
  if (airTempStats.unbiased_stdev() >= airTempStats.average() * tolerance)
    return false;
  if (humidityStats.unbiased_stdev() >= humidityStats.average() * tolerance)
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

String Reading::toString(bool fah) {
  String line = "";
  if (fah)
    line = line + cToF(groundTemperature) + "F," + moisture + "," + pH + "," + cToF(airTemperature) + "F," + humidity + "," + lux;
  else
    line = line + groundTemperature + "C," + moisture + "," + pH + "," + airTemperature + "C," + humidity + "," + lux;
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
  while ( serial->available() > 0)    // Clears the buffer
    serial->read();

  reading->pH = sensorString.toFloat();
}

void pH::setUp() {}

void pH::setReading(Reading *r) {
  reading = r;

}

void pH::calibrate() {
  serial->print("Cal,mid,7.00\r");
  while ( serial->available() > 0)    // Clears the buffer
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







