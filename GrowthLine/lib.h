/* Libraries needed for Light Sensor  */
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <pgmspace.h>

/* Dependencies to the Temp/Humid DHT03 Sensor */
#include "dht.h"

/* Librariy for pH circuit */
#include <SoftwareSerial.h>

/* Library needed for Temperature/Moisture Sensor (SHT10)  */
#include <SHT1x.h>

/* Reading class. It holds all the values read from the sensors */
class Reading {
  public:
    float lux;
    float airTemperature;
    float humidity;
    float pH;
    float groundTemperature;
    float moisture;

    Reading( float, float, float, float, float, float );
    Reading();
};

/* Abstract class, all sensors inherit from this */
class Sensor {
  public:
    Reading *reading;
    Sensor() {}
    virtual ~Sensor() = 0;
    virtual void read() = 0;
    virtual void setUp() = 0;
};

/* Light sensor class */
class LightSensor : public Sensor {
  public:
    Adafruit_TSL2561_Unified luxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

    LightSensor(Reading *r);
    ~LightSensor();
    void read();
    void setUp();
};

/* Air Temperature/Humidity Class */
class TempHumid : public Sensor {
    int pin;
    dht DHT;
    int err;
  public:
    TempHumid(Reading *r, int);
    ~TempHumid();
    void read();
    void setUp();        
};

/* pH Sensor Class */
class pH : public Sensor {
  int rx;
  int tx;
  SoftwareSerial *serial;
  public:
    pH(Reading *r, int re, int t);
    ~pH();
    void read();
    void setUp();
};

/* Ground Temperature/Moisture Class */  // Not Done Yet
class TempMoist : public Sensor {
  int dataPin;
  int clockPin;
  SHT1x *sht1x;
  public:
    TempMoist(Reading *r, int d, int c);
    ~TempMoist();
    void read();
    void setUp();
};




