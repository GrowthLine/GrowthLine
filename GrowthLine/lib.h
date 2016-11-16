#define READY_STATE 0
#define WARMUP_STATE 1
#define READ_STATE 2
#define SAVE_STATE 3
#define MENU_STATE 4
#define CALIBRATE_STATE 5
#define SHUTDOWN_STATE 6

#define PH_RECEIVE_PIN 2
#define PH_TRANSMIT_PIN 3
#define TEMP_HUMID_PIN 5
#define TEMP_MOIST_DATA_PIN 6
#define TEMP_MOIST_CLOCK_PIN 7

#define LUX_SENSOR_ID 0
#define TEMP_HUMID_SENSOR_ID 1
#define PH_SENSOR_ID 2
#define TEMP_MOIST_SENSOR_ID 3

// For the touch controller
#define STMPE_CS 8
// For the TFT screen
#define TFT_DC 9
#define TFT_CS 10

// Touch screen min/max co-ordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// Button constants
#define BTN_NONE 0
#define BTN_NW   1
#define BTN_NE   2
#define BTN_SW   3
#define BTN_SE   4

/* Library needed for the Queue List */
#include <QueueList.h>

/* Library needed for Touch LCD */
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_STMPE610.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

/* Libraries needed for Light Sensor  */
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <pgmspace.h>

/* Dependencies to the Temp/Humid DHT03 Sensor */
#include <dht.h>

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
    String toString();
};

/* Abstract class, all sensors inherit from this */
class Sensor {
  public:
    Reading *reading;
    uint8_t ID;
    Sensor() {}
    virtual ~Sensor() = 0;
    virtual void read() = 0;
    virtual void setUp() = 0;
    virtual void setReading(Reading *r);
};

/* Light sensor class */
class LightSensor : public Sensor {
  public:
    Adafruit_TSL2561_Unified luxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

    LightSensor();
    ~LightSensor();
    void read();
    void setUp();
    void setReading(Reading *r);
};

/* Air Temperature/Humidity Class */
class TempHumid : public Sensor {
    uint8_t pin;
    dht DHT;
    uint8_t err;
  public:
    TempHumid(uint8_t);
    ~TempHumid();
    void read();
    void setUp();
    void setReading(Reading *r);
};

/* pH Sensor Class */
class pH : public Sensor {
    uint8_t rx;
    uint8_t tx;
    SoftwareSerial *serial;
  public:
    pH(uint8_t, uint8_t);
    ~pH();
    void read();
    void setUp();
    void setReading(Reading *r);
    void calibrate(uint8_t);
};

/* Ground Temperature/Moisture Class */
class TempMoist : public Sensor {
    uint8_t dataPin;
    uint8_t clockPin;
    SHT1x *sht1x;
  public:
    TempMoist(uint8_t, uint8_t);
    ~TempMoist();
    void read();
    void setUp();
    void setReading(Reading *r);
};

/* Sensors Class */
class Sensors {
    QueueList<Sensor*> sensors;
  public:
    Sensors();
    void addSensor(Sensor *s);
    void setupSensors();
    Reading* getReading();
    Sensor* getSensor(uint8_t id);
};



