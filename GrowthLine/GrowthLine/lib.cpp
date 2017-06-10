#include "lib.h"

LightSensor::LightSensor() : Sensor(LUX_SENSOR_ID) {}
LightSensor::~LightSensor() {}

List<Read> LightSensor::read() {
    sensors_event_t event;
    luxSensor.getEvent(&event);
    List<Read> reads;
    Read read(event.light, ReadType::LUX);
    reads.add(read);
    return reads;
}

void LightSensor::setUp() {
    luxSensor.enableAutoRange(true);
    luxSensor.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
}

void LightSensor::calibrate() {}

TempHumid::TempHumid(uint8_t p) : Sensor(TEMP_HUMID_SENSOR_ID), pin(p) {
    DHT = new DHT_Unified(p, DHT22);
}

TempHumid::~TempHumid() { delete DHT; }

List<Read> TempHumid::read() {
    sensors_event_t event;
    List<Read> reads;

    DHT->temperature().getEvent(&event);
    Read airRead(event.temperature, ReadType::AIR_TEMP);
    DHT->humidity().getEvent(&event);
    Read airHumid(event.relative_humidity, ReadType::HUMIDITY);
    reads.add(airRead);
    reads.add(airHumid);
    return reads;
}

void TempHumid::setUp() {}
void TempHumid::calibrate() {}

pH::pH(uint8_t re, uint8_t t) : Sensor(PH_SENSOR_ID), rx(re), tx(t) {
    serial = new SoftwareSerial(rx, tx);
    serial->begin(9600);
    serial->print("RESPONSE,0\r");
}

pH::~pH() { delete serial; }

List<Read> pH::read() {
    serial->print("R\r");
    String sensorString = "";
    while ( serial->available() > 0 ) {
        char inchar = (char)serial->read();
        if (inchar == '\r')
            break;
            sensorString += inchar;
    }
    while ( serial->available() > 0) serial->read();   // Clears the buffer
  
    List<Read> reads;
    Read read(sensorString.toFloat(), ReadType::PH);
    reads.add(read);
    return reads;
}

void pH::setUp() {}
void pH::calibrate() {
    serial->print("Cal,mid,7.00\r");
    while (serial->available() > 0) serial->read();
}

TempMoist::TempMoist(uint8_t d, uint8_t c) : Sensor(TEMP_MOIST_SENSOR_ID), dataPin(d), clockPin(c) {
    sht1x = new SHT1x(dataPin, clockPin);
}

TempMoist::~TempMoist() { delete sht1x; }

List<Read> TempMoist::read() {
    List<Read> reads;
    Read grndTemp(sht1x->readTemperatureC(), ReadType::GROUND_TEMP);
    Read humid(sht1x->readHumidity(), ReadType::HUMIDITY);
    reads.add(grndTemp);
    reads.add(humid);
    return reads;
}

void TempMoist::setUp() {}
void TempMoist::calibrate() {}

uint8_t getQuadrantFromPoint(TS_Point *p) {
    if ( p->x > TS_MAXX / 2 && p->y < TS_MAXY / 2) {            // North-West touch
        return BTN_NW;
    }
    if ( p->x < TS_MAXX / 2 && p->y < TS_MAXY / 2) {            // South-West touch
        return BTN_SW;
    }
    if ( p->x < TS_MAXX / 2 && p->y > TS_MAXY / 2) {            // South-East touch
        return BTN_SE;
    }
    if ( p->x > TS_MAXX / 2 && p->y > TS_MAXY / 2) {            // North-East Touch
        return BTN_NE;
    }
    return BTN_NONE;
}

bool stableReadings(List<Reading>& readings) {
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

    for (readings.begin(); readings.isNotAtEnd(); readings.advance()) {
        Reading reading = readings.getAtCursor();
        phStats.add(reading.getReadValueByType(ReadType::PH));
        grndTempStats.add(reading.getReadValueByType(ReadType::GROUND_TEMP));
        moistureStats.add(reading.getReadValueByType(ReadType::MOISTURE));
        airTempStats.add(reading.getReadValueByType(ReadType::AIR_TEMP));
        humidityStats.add(reading.getReadValueByType(ReadType::HUMIDITY));
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