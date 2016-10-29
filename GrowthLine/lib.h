/* Libraries needed for Light Sensor  */
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <pgmspace.h>


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
class LightSensor : Sensor {
  public:
    Adafruit_TSL2561_Unified luxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

    LightSensor(Reading *r);
    ~LightSensor();
    void read();
    void setUp();
};

/* Air Temperature/Humidity Class */
//class TempHumid : sensor {
//    int pin;
//    dht DHT;
//    int err;
//  public:
//    TempHumid(Reading *r);
//    ~LightSensor();
//    void read();
//    void setUp();        
//};





