#include "lib.h"

/* Variable Declarations */
uint8_t deviceState;                        // State of device
const uint8_t NUMBER_OF_READINGS = 5;       // Number of readings we will hold before discarding old one

QueueList<Reading*> readings;
Sensors sensors;

Adafruit_STMPE610 touch = Adafruit_STMPE610(8);

void setup() {
  Serial.begin(9600);
  readings.setPrinter(Serial);
  deviceState = READY_STATE;

  /* Add the sensors to our Sensors object */
  sensors.addSensor(new LightSensor());
  sensors.addSensor(new TempHumid(TEMP_HUMID_PIN));
  sensors.addSensor(new pH(PH_RECEIVE_PIN, PH_TRANSMIT_PIN));
  sensors.addSensor(new TempMoist(TEMP_MOIST_DATA_PIN, TEMP_MOIST_CLOCK_PIN));

  /* Setup the sensors */
  sensors.setupSensors();

  /* Screen setup place here */
}

void loop() {
  switch (deviceState) {
    case READY_STATE:
      break;
    case WARMUP_STATE:
      break;
    case READ_STATE:
      if (readings.count() > NUMBER_OF_READINGS)
        readings.pop();
      readings.push( sensors.getReading() );
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
  delay(1000);
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

//      Serial.print("The lux is: "); Serial.println(readings.peek()->lux);
//      Serial.print("The air temperature is: "); Serial.println(readings.peek()->airTemperature);
//      Serial.print("The humidity is: "); Serial.println(readings.peek()->humidity);
//      Serial.print("The pH is: "); Serial.println(readings.peek()->pH);
//      Serial.print("The ground temperature is: "); Serial.println(readings.peek()->groundTemperature);
//      Serial.print("The moisture is: "); Serial.println(readings.peek()->moisture);
//      Serial.println("\n\n");














