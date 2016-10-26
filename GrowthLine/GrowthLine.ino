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

Adafruit_TSL2561_Unified luxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); // Lux Sensor
int deviceState = READY_STATE;


void setup() {
  Serial.begin(9600);
  /* Configure Lux Sensor */
  luxSensor.enableAutoRange(true);    // switches automatically between the different gains
  luxSensor.setIntegrationTime(TSL2561_INTEGRATIONTIME_420MS);  // slowest but most accurate

}

void loop() {
  switch (deviceState) {
    case READY_STATE:
      mainMenu();
      break;
    case WARMUP_STATE:
      break;
    case READ_STATE:
      break;
    case SAVE_STATE:
      break;
    case MENU_STATE:
      break;
    case CALIBRATE_STATE:
      break;
    case SHUTDOWN_STATE:
      break;
    default:
      break;
  }
  
  void mainMenu() {
    fillRect( 20, 20,120, 80, ILI9341_GREEN);
    fillRect(170, 20,120, 80, ILI9341_YELLOW);
    fillRect( 20,140,120, 80, ILI9341_ORANGE);
  }

}
