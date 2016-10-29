#include "GrowthLine.h"
#include "lib.h"

/* Library needed for Touch LCD */
#include <Adafruit_ILI9341.h>

/* Variable Declarations */
int deviceState ;                        // State of device

Reading reading;
LightSensor lux(&reading);

void setup() {
  Serial.begin(9600);
  deviceState = READY_STATE;
  // Testing
  lux.setUp();
  Serial.println("Setup is complete");
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
















