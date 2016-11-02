#include "GrowthLine.h"
#include "lib.h"

/* Libraries needed for Touch LCD */
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

/* Variable Declarations */
int deviceState, button;                        // State of device
bool redraw, have_readings;
const int totalSensors = 4;

Reading reading;
Sensor *sensors[totalSensors];
LightSensor lux(&reading);
Adafruit_STMPE610 *ts;
Adafruit_ILI9341 *tft;
TS_Point *p;

void setup() {
  Serial.begin(9600);
  deviceState   = READY_STATE;
  button        = BTN_NONE;
  redraw        = true;
  have_readings = false;

  /* Add the sensors to our array of Sensors */
  sensors[0] = new LightSensor(&reading);
  sensors[1] = new TempHumid(&reading, tempHumidPin);
  sensors[2] = new pH(&reading, phReceivePin, phTransmitPin);
  sensors[4] = new TempMoist(&reading, -1, -1);     // TempMoist Class not done
  
  tft = new Adafruit_ILI9341(TFT_CS, TFT_DC);
  ts = new Adafruit_STMPE610(STMPE_CS);
  p = new TS_Point();


  /* Setup the sensors */
 // for(int i=0; i < totalSensors; i++)
 //   sensors[i]->setUp();
  bool started = false;
  while (!started) {
    if (ts->begin()) {
      started = true;
    } else {
      Serial.println("Touchscreen controller start failure!");
    }
  }
  tft->setRotation(1);
  Serial.println("Setup is complete");
}

void loop() {
  switch (deviceState) {
    case READY_STATE:
      if (redraw) {
        draw_MainMenu();
        redraw = false;
      }
      button = check_MainMenu();
      switch (button) {
        case BTN_NW:
          deviceState = WARMUP_STATE;
          redraw = true;
          break;
        case BTN_NE:
          deviceState = MENU_STATE;
          redraw = true;
          break;
        case BTN_SW:
          deviceState = SAVE_STATE;
          redraw = true;
          break;
        default:
          break;
      }
      break;
    case WARMUP_STATE:
      if (redraw) {
        draw_WarmUpScreen();
        redraw = false;
      }
      // Set deviceState to READ_STATE
      break;
    case READ_STATE:
      // Perhaps we should get sensor values first.
      if (redraw) {
        draw_ReadScreen();
        redraw = false;
      }
      break;
    case SAVE_STATE:
      break;
    case MENU_STATE:
      draw_MenuScreen();
      break;
    case CALIBRATE_STATE:
      break;
    case SHUTDOWN_STATE:
      break;
    default:
      break;
  }
}

void draw_MainMenu() {
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_GREEN );
  tft->fillRect(170, 20, 120, 80, ILI9341_YELLOW);
  tft->fillRect( 20, 140, 120, 80, ILI9341_ORANGE);
  tft->setTextSize(2);
  // Write text on button 1
  tft->setCursor( 55, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN );
  tft->println("Read");
  // Write text on button 2
  tft->setCursor(205, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_YELLOW);
  tft->println("Menu");
  // Write text on button 3
  tft->setCursor( 55,172);
  tft->setTextColor( ILI9341_BLACK, ILI9341_ORANGE);
  tft->println("Save");
}

int check_MainMenu() {
  if (ts->bufferEmpty()) {
    //Serial.println("No button press.\n");
    return BTN_NONE;
  }
  p = &ts->getPoint();
  
  // Scale the coordinates
  p->x = map(p->x, TS_MINX, TS_MAXX, 0, tft->width());
  p->y = map(p->y, TS_MINY, TS_MAXY, 0, tft->height());
  
  // First button check: 'Read'
  if ((p->x >=  20 && p->x <= 140) && (p->y >=  20 && p->y <= 100)) {
    return BTN_NW;
  }
  // Second button check: 'Menu'
  if ((p->x >= 170 && p->x <= 290) && (p->y >=  20 && p->y <= 100)) {
    return BTN_NE;
  }
  // Third button check: 'Save'
  if ((p->x >=  20 && p->x <= 140) && (p->y >= 140 && p->y <= 260)) {
    return BTN_SW;
  }
  return BTN_NONE;
   
}

void draw_WarmUpScreen() {
  // Blank screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw box
  tft->fillRect( 20, 20, 120, 80, ILI9341_YELLOW);
  // Draw text
  tft->setCursor( 40, 50);
  tft->setTextColor( ILI9341_BLACK, ILI9341_YELLOW);
  tft->println("Warming up");
}

void draw_ReadScreen() {
  // Blank the screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_RED);
  tft->fillRect(160, 20,140,100, ILI9341_YELLOW);
  tft->fillRect( 20,120,140,100, ILI9341_GREEN);
  tft->fillRect(160,120,140,100, ILI9341_BLUE);
  tft->setTextSize(2);
  // Draw text
  tft->setCursor( 55, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED);
  tft->println("Stop");
  // Draw reading 1
  // Draw reading 2
  // Draw reading 3
}

void draw_MenuScreen() {
  // Blank the screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20,120, 60, ILI9341_RED);
  tft->fillRect(180, 20,120, 60, ILI9341_YELLOW);
  tft->fillRect( 20, 90,280, 40, ILI9341_MAGENTA);
  tft->fillRect( 20,140,280, 40, ILI9341_MAGENTA);
  tft->fillRect( 20,190,280, 40, ILI9341_MAGENTA);
  tft->setTextSize(2);
  // Write text on button 1
  tft->setCursor( 55, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED );
  tft->println("Back");
  // Celsius or Fahrenheit
  // Shutdown
  // Calibrate
  // Logs
}


\
