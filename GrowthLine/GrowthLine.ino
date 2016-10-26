#include <GrowthLine.h>
void setup() {
  Serial.begin(9600);
  /* Configure Lux Sensor */
  luxSensor.enableAutoRange(true);    // switches automatically between the different gains
  luxSensor.setIntegrationTime(TSL2561_INTEGRATIONTIME_420MS);  // slowest but most accurate

  deviceState = READ_STATE;   // set initial device state
}

void loop() {
  switch (deviceState) {
    case READY_STATE:
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
