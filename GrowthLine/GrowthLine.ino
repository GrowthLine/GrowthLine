#include "GrowthLine.h"
#include "lib.h"

/* Library needed for Touch LCD */
#include <Adafruit_ILI9341.h>

/* Variable Declarations */
int deviceState;                        // State of device
int button;
const int totalSensors = 4;

Reading reading;
Sensor *sensors[totalSensors];
LightSensor lux(&reading);
Adafruit_ILI9341 display;

void setup() {
  Serial.begin(9600);
  deviceState = READY_STATE;
  button = BTN_NONE;

  /* Add the sensors to our array of Sensors */
  sensors[0] = new LightSensor(&reading);
  sensors[1] = new TempHumid(&reading, tempHumidPin);
  sensors[2] = new pH(&reading, phReceivePin, phTransmitPin);
  sensors[3] = new TempMoist(&reading, -1, -1);     // TempMoist Class not done

  tft = new Adafruit_ILI9341(TFT_CS, TFT_DC);
  /* Setup the sensors */
  for (int i = 0; i < totalSensors; i++)
    sensors[i]->setUp();
  tft.begin();
  Serial.println("Setup is complete");
}

void loop() {
  for ( int i = 0; i < totalSensors; i++)
    sensors[i]->read();
  Serial.print("The lux is: "); Serial.println(reading.lux);
  Serial.print("The air temperature is: "); Serial.println(reading.airTemperature);
  Serial.print("The humidity is: "); Serial.println(reading.humidity);
  Serial.print("The pH is: "); Serial.println(reading.pH);
  Serial.println("\n\n");
  delay(5000);
  switch (deviceState) {
    case READY_STATE:
      // draw_MainMenu();
      // button = check_MainMenu();
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

/**
    Tries to grab a point, returns null if none present.
 **/
TS_Point getTouch() {
  // If the touchbuffer is empty, then return
  if (ts.bufferEmpty()) {
    return null;
  }
  // Grab the next touchpoint from the buffer
  TS_Point p = ts.getPoint();

  // Scale the coordinates
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  // Return the value
  return p;
}

void draw_MainMenu() {
  // Blank screen
  tft.fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft.fillRect( 20, 20, 120, 80, ILI9341_GREEN );
  tft.fillRect(170, 20, 120, 80, ILI9341_YELLOW);
  tft.fillRect( 20, 140, 120, 80, ILI9341_ORANGE);
  // Write text on button 1
  tft.setCursor( 50, 50);
  tft.setTextColor( ILI9341_BLACK, ILI9341_GREEN );
  tft.println("Read");
  // Write text on button 2
  tft.setCursor(200, 50);
  tft.setTextColor( ILI9341_BLACK, ILI9341_YELLOW);
  tft.println("Menu");
  // Write text on button 3
  tft.setCursor( 50, 170);
  tft.setTextColor( ILI9341_BLACK, ILI9341_ORANGE);
  tft.println("Save");
}

int check_MainMenu() {
  TS_Point p = getTouch();
  if (p == null) {
    return BTN_NONE;
  }
  // First button check: 'Read'
  if ((p.x >=  20 && p.x <= 140) && (p.y >=  20 && p.y <= 100)) {
    return BTN_NW;
  }
  // Second button check: 'Menu'
  if ((p.x >= 170 && p.x <= 290) && (p.y >=  20 && p.y <= 100)) {
    return BTN_NE;
  }
  // Third button check: 'Save'
  if ((p.x >=  20 && p.x <= 140) && (p.y >= 140 && p.y <= 260)) {
    return BTN_SW;
  }
  return BTN_NONE;

}

void draw_WarmUpScreen() {
  // Blank screen
  tft.fillScreen(ILI9341_BLACK);
  // Draw box
  tft.fillRect( 20, 20, 120, 80, ILI9341_YELLOW);
  // Draw text
  tft.setCursor( 40, 50);
  tft.setTextColor( ILI9341_BLACK, ILI9341_YELLOW);
  tft.println("Warming up");
}
















