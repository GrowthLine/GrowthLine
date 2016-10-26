#include "GrowthLine.h"

/* Libraries needed for Light Sensor  */
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <pgmspace.h>

/* Library needed for Temperature/Moisture Sensor (SHT10)  */
#include <SHT1x.h>

/* Library needed for Touch LCD */
#include <Adafruit_ILI9341.h>

/* Librariy for pH circuit */
#include <SoftwareSerial.h>

/* Dependencies to the Temp/Humid DHT03 Sensor */
#include "dht.h"

/* Variable Declarations */
Adafruit_TSL2561_Unified luxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); // Lux Sensor
dht tempHumidsensor;                  // Temperatue & Humid Sensor (DHT03)
SoftwareSerial phSerial( phReceivePin, phTransmitPin);    // Serial of the pH Sensor
int deviceState ;                     // State of device
String outpH = "";                    // Message that will be given to the pH sensor
String inpH = "";                     // hold the message incoming from the pH sensor




void setup() {
  Serial.begin(9600);

  deviceState = READY_STATE;
}

void loop() {
  switch (deviceState) {
    case READY_STATE:
      //mainMenu();
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
}

//void mainMenu() {
//  fillRect( 20, 20, 120, 80, ILI9341_GREEN );
//  fillRect(170, 20, 120, 80, ILI9341_YELLOW);
//  fillRect( 20, 140, 120, 80, ILI9341_ORANGE);
//  setCursor( 50, 50);
//  setTextColor( ILI9341_BLACK, ILI9341_GREEN);
//}
//
//void warmUpScreen() {
//  fillRect( 20, 20, 120, 80, ILI9341_YELLOW);
//}

void configureSensors() {
  /* Configure Lux Sensor */
  //luxSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); // Lux Sensor
  luxSensor.enableAutoRange(true);    // switches automatically between the different gains
  luxSensor.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  // slowest but most accurate

  /* Configure pH Sensor */
  outpH.reserve(10);
  inpH.reserve(30);
  phSerial.begin(9600);

}

