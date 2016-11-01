#include "GrowthLine.h"
#include "lib.h"

/* Library needed for Touch LCD */
#include <Adafruit_ILI9341.h>

/* Variable Declarations */
int deviceState;                        // State of device
const int totalSensors = 4;

Reading reading;
Sensor *sensors[totalSensors];
LightSensor lux(&reading);

void setup() {
  Serial.begin(9600);
  deviceState = READY_STATE;

  /* Add the sensors to our array of Sensors */
  sensors[0] = new LightSensor(&reading);
  sensors[1] = new TempHumid(&reading, tempHumidPin);
  sensors[2] = new pH(&reading, phReceivePin, phTransmitPin);
  sensors[3] = new TempMoist(&reading, 12, 13);     // TempMoist Class not done
  
  /* Setup the sensors */
  for (int i = 0; i < totalSensors; i++)
    sensors[i]->setUp();
  Serial.println("Setup is complete");
}

void loop() {
  for ( int i = 0; i < totalSensors; i++)
    sensors[i]->read();
  Serial.print("The lux is: "); Serial.println(reading.lux);
  Serial.print("The air temperature is: "); Serial.println(reading.airTemperature);
  Serial.print("The humidity is: "); Serial.println(reading.humidity);
  Serial.print("The pH is: "); Serial.println(reading.pH);
  Serial.print("The ground temperature is: "); Serial.println(reading.groundTemperature);
  Serial.print("The moisture is: "); Serial.println(reading.moisture);
  Serial.println("\n\n");
  delay(5000);
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
















