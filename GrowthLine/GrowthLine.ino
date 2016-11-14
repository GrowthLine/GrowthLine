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

void setup() {
  Serial.begin(9600);
  deviceState   = READY_STATE;
  button        = BTN_NONE;
  have_readings = false;

  /* Add the sensors to our array of Sensors */
  sensors[0] = new LightSensor(&reading);
  sensors[1] = new TempHumid(&reading, tempHumidPin);
  sensors[2] = new pH(&reading, phReceivePin, phTransmitPin);
  sensors[4] = new TempMoist(&reading, -1, -1);     // TempMoist Class not done
  
  tft = new Adafruit_ILI9341(TFT_CS, TFT_DC);
  ts = new Adafruit_STMPE610(STMPE_CS);


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
  tft->begin();
  tft->setRotation(1);
  redraw = true;
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
      if (redraw) {
        draw_SaveScreen();
        redraw = false;
      }
      break;
    case MENU_STATE:
      if (redraw) {
        draw_MenuScreen();
        redraw = false;
      }
      break;
    case CALIBRATE_STATE:
      break;
    case SHUTDOWN_STATE:
      if (redraw) {
        draw_ShutdownScreen();
        redraw = false;
      }
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
  uint16_t x, y;
  uint8_t z;
  if (ts->bufferEmpty()) {
    //Serial.println("No button press.\n");
    return BTN_NONE;
  }
  ts->readData(&x, &y, &z);
  
  
  // Scale the coordinates
  x = map(x, TS_MINX, TS_MAXX, 0, tft->width());
  y = map(y, TS_MINY, TS_MAXY, 0, tft->height());
  
  // First button check: 'Read'
  if ((x >=  20 && x <= 140) && (y >=  20 && y <= 100)) {
    return BTN_NW;
  }
  // Second button check: 'Menu'
  if ((x >= 170 && x <= 290) && (y >=  20 && y <= 100)) {
    return BTN_NE;
  }
  // Third button check: 'Save'
  if ((x >=  20 && x <= 140) && (y >= 140 && y <= 220)) {
    return BTN_SW;
  }
  return BTN_NONE;
   
}

void draw_WarmUpScreen() {
  tft->setTextSize(3);
  // Blank screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw text
  tft->setCursor( 60, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("Warming up");
}

void draw_SaveScreen() {
  tft->setTextSize(3);
  // Blank screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw text
  tft->setCursor( 62, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("Saving");
}

void draw_ShutdownScreen() {
  tft->setTextSize(3);
  // Blank screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw text
  tft->setCursor( 60, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("Shutting Down");
}

void draw_ReadScreen() {
  tft->setTextSize(2);
  // Blank the screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 200, 80, ILI9341_RED);
  //tft->fillRect(160, 20,140,100, ILI9341_YELLOW);
  //tft->fillRect( 20,120,140,100, ILI9341_GREEN);
  //tft->fillRect(160,120,140,100, ILI9341_BLUE);
  tft->setTextSize(2);
  // Draw text
  tft->setCursor( 55, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED);
  tft->println("Stop");
  // Draw reading 1
  /*
  tft->setCursor(200, YPOS);
  tft->print("pH      :");
  tft->println(readings.peek()->pH);
  */
  // Draw reading 2
  /*
  tft->setCursor(200, YPOS);
  tft->print("Moisture:");
  tft->println(readings.peek()->moisture);
  */
  // Draw reading 3
  /*
  tft->setCursor(200, YPOS);
  tft->print("Temp   :");
  tft->println(readings.peek()->groundTemperature);
  */
}

void draw_MenuScreen() {
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_YELLOW );
  tft->fillRect(180, 20, 120, 80, ILI9341_MAGENTA);
  tft->fillRect( 20,140, 120, 80, ILI9341_MAGENTA);
  tft->fillRect(180,140, 120, 80, ILI9341_MAGENTA);
  tft->setTextSize(2);
  // Write text on button 1
  tft->setCursor( 55, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN  );
  tft->println("Back");
  // Set text color for following buttons
  tft->setTextColor( ILI9341_BLACK, ILI9341_MAGENTA);
  // Write text on button 2
  tft->setCursor(205, 52);
  tft->println("C/F");
  // Write text on button 3
  tft->setCursor( 55,172);
  tft->println("Logs");
  // Write text on button 4
  tft->setCursor(200,172);
  tft->println("Calibrate");
}

