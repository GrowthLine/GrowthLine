// Libraries needed for Light Sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <pgmspace.h>
// Libraries needed for Temperature/Moisture Sensor (SHT10)
#include <SHT1x.h>
// Libraries needed for Touch LCD
#include <Adafruit_ILI9341.h>


#define READY_STATE = 0;
#define WARMUP_STATE = 1;
#define READ_STATE = 2;
#define SAVE_STATE = 3;
#define MENU_STATE = 4;
#define CALIBRATE_STATE = 5;
#define SHUTDOWN_STATE = 6;

Adafruit_TSL2561_Unified luxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); // Lux Sensorint deviceState = READY_STATE;
