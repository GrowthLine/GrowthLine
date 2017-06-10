/* Changable Parameters */
#define READING_FREQUENCY 2000                  // Length in milliseconds before each reading
#define NUMBER_OF_READINGS 5                    // Number of readings we will hold before discarding old one
#define WARMUP_LENGTH 5000                      // length of the warm up in milliseconds
#define WARMUP_TOLERANCE 0.05                   // Value used to set how how apart should values be from average
#define GOOD_LOW_PH 5.5                         // Determines lowest "good" pH value for soil
#define GOOD_HIGH_PH 7.0                        // Determines highest "good" pH value for soil
#define GOOD_LOW_GROUND_TEMPERATURE 21.1111     // Determines lowest "good" ground temperature value for soil
#define GOOD_HIGH_GROUND_TEMPERATURE 35.0000    // Determines highest "good" ground temperature value for soil

// Different device states
#define READY_STATE 0
#define WARMUP_STATE 1
#define READ_STATE 2
#define SAVE_STATE 3
#define MENU_STATE 4
#define CALIBRATE_STATE 5
#define SHUTDOWN_STATE 6
#define LOG_STATE 7
#define SETTINGS_STATE 8

// PINs for the sensors
#define PH_RECEIVE_PIN 2
#define PH_TRANSMIT_PIN 3
#define SD_CS_PIN 4
#define TEMP_HUMID_PIN 5
#define TEMP_MOIST_DATA_PIN 6
#define TEMP_MOIST_CLOCK_PIN 7

// IDs given to the different sensors
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
#define BTN_NE   1
#define BTN_NW   2
#define BTN_SW   3
#define BTN_SE   4

// Reading Classifications
#define BAD_LOW   0
#define GOOD      1
#define BAD_HIGH  2
#define NOT_KNOWN   3

/* Libraries needed from the GrowthLine Project Libraries */
#include <Utilities.h>
#include <Measurements.h>
#include <Sensor.h>
#include <Logger.h>

/* Library needed for statistic calculations */
#include <Statistic.h>

/* Libraries needed for Touch LCD */
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_STMPE610.h>
#include <Adafruit_ILI9341.h>

/* Libraries needed for Light Sensor  */
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

/* Dependencies to the Temp/Humid DHT03 Sensor */
#include <DHT_U.h>

/* Librariy for pH circuit */
#include <SoftwareSerial.h>

/* Library needed for Temperature/Moisture Sensor (SHT10)  */
#include <SHT1x.h>

/* SD Library */
#include <SD.h>

/* Light sensor class */
class LightSensor : public Sensor {
  public:
    Adafruit_TSL2561_Unified luxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

    LightSensor();
    ~LightSensor();
    List<Read> read();
    void setUp();
    void calibrate();
};

/* Air Temperature/Humidity Class */
class TempHumid : public Sensor {
private:
    uint8_t pin;
    DHT_Unified *DHT;
    uint8_t err;
  public:
    TempHumid(uint8_t p);
    ~TempHumid();
    List<Read> read();
    void setUp();
    void calibrate();
};

/* pH Sensor Class */
class pH : public Sensor {
private:
    uint8_t rx;
    uint8_t tx;
    SoftwareSerial *serial;
  public:
    pH(uint8_t re, uint8_t t);
    ~pH();
    List<Read> read();
    void setUp();
    void calibrate();
};

/* Ground Temperature/Moisture Class */
class TempMoist : public Sensor {
    uint8_t dataPin;
    uint8_t clockPin;
    SHT1x *sht1x;
  public:
    TempMoist(uint8_t d, uint8_t c);
    ~TempMoist();
    List<Read> read();
    void setUp();
    void calibrate();
};

/* General Functions */
/* Finds the quadrant touched by the user on the screen, wide screen orientation */
uint8_t getQuadrantFromPoint(TS_Point *p);

/* analyzse the readings list and see if the readings are stable */
bool stableReadings(List<Reading>& readings);

/* Determines pH value classification */
uint8_t phStatus(float ph);

/* Determines ground temperature value classification */
uint8_t groundTempStatus(float t);
