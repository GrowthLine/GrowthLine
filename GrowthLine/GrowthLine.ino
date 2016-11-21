#include "lib.h"

/* Changable Parameters */
#define READING_FREQUENCY 2000                  // Length in milliseconds before each reading
#define NUMBER_OF_READINGS 5                    // Number of readings we will hold before discarding old one
#define WARMUP_LENGTH 5000                      // length of the warm up in milliseconds

/* Variable Declarations */
QueueList<Reading> readings;                   // List used to hold reading objects. Structured in a queue.
Sensors sensors;                                // Object that will hold and manage all other sensors
int deviceState;                                // State of device
bool fahrenheit;                                // Saves user information about temperature unit
bool redraw;                                    // Flag used to know when to redraw a screen
bool saveEnable;                                // Used to disable the save button (in case SD card is not inserted)
bool tempChange;                                // Flag used to ID when to redraw Temp buttons
unsigned long milliseconds;                     // saves milliseconds. Used with millis() to check lenths of time
unsigned int readingNumber;                     // holds the current value that will be used to write to SD card
unsigned int logFileNumber;                     // holds the current log file number
Adafruit_STMPE610 *ts;                          // pointer to a touch screen object
Adafruit_ILI9341 *tft;                          // pointer to a display object

void setup() {
  Serial.begin(9600);
  readings.setPrinter(Serial);
  deviceState   = READY_STATE;
  redraw        = true;
  fahrenheit    = false;
  saveEnable    = true;
  tempChange    = true;

  /* Add the sensors to our Sensors object */
  sensors.addSensor(new LightSensor());
  sensors.addSensor(new TempHumid(TEMP_HUMID_PIN));
  sensors.addSensor(new pH(PH_RECEIVE_PIN, PH_TRANSMIT_PIN));
  sensors.addSensor(new TempMoist(TEMP_MOIST_DATA_PIN, TEMP_MOIST_CLOCK_PIN));

  /* Create our display and touch objects */
  tft = new Adafruit_ILI9341(TFT_CS, TFT_DC);
  ts = new Adafruit_STMPE610(STMPE_CS);

  /* Setup the sensors */
  sensors.setupSensors();

  /* Setting up the SD card */
  File settingsFile;
  if (! SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    saveEnable = false;
  }
  else
    settingsFile = SD.open("settings.txt");


  char tempSetting = ' ';
  while (settingsFile.available()) {
    tempSetting = settingsFile.read();

  }
  /* check that touch screen is started propperly */
  while (true) {
    if (ts->begin()) {
      break;
    } else {
      Serial.println("Touchscreen controller start failure!");
    }
  }
  tft->begin();
  tft->setRotation(1);
  Serial.println("Setup is complete");
}

/*
   The if (redraw) prevents the entire screen from
   being redrawn every loop() cycle. This prevents
   flickering.
*/
void loop() {
    // Determine if the screen was touched and on which quadrant
    TS_Point touchedPoint;
    uint8_t touchedQuadrant = BTN_NONE;
    while (ts->touched()) {
      touchedPoint = ts->getPoint();
      if ( touchedPoint.z < 75 )
        touchedQuadrant = getQuadrantFromPoint(&touchedPoint);
    }
  
    // State Machine of the device
    switch (deviceState) {
      case READY_STATE:
        if (redraw) {
          draw_MainMenu();
          redraw = false;
        }
        switch (touchedQuadrant) {
          case BTN_NE:
            deviceState = MENU_STATE;
            redraw = true;
            break;
          case BTN_NW:
            deviceState = WARMUP_STATE;
            redraw = true;
            break;
        }
        break;
      case WARMUP_STATE:
        if (redraw) {
          draw_WarmUpScreen();
          redraw = false;
        }
        milliseconds = millis();
        while (millis() - milliseconds < WARMUP_LENGTH)    // Get readings without saving for several seconds without saving to warm up
          sensors.getReading();
        deviceState = READ_STATE;
        redraw = true;
        break;
      case READ_STATE:
        if (redraw) {
          draw_ReadScreen();
          redraw = false;
        }
        switch (touchedQuadrant) {
          case BTN_NE:
            if (saveEnable) { // if save button is pressed, save and go back to main menu
              deviceState = SAVE_STATE;
              redraw = true;
            }
            break;
          case BTN_NW:      // if stop button is pressed, do not save and go back to main menu
            deviceState = READY_STATE;
            redraw = true;
            break;
        }
        if ( millis() - milliseconds > READING_FREQUENCY) {   // updates displayed reading
          if (readings.count() > NUMBER_OF_READINGS)
            readings.pop();
          readings.push( sensors.getReading() );
          update_Readings();
          milliseconds = millis();
        }
        break;
      case SAVE_STATE:
        if (redraw) {
          draw_SaveScreen();
          redraw = false;
        }
        // ******** Save to SD card code here ******
        deviceState = READY_STATE;
        redraw = true;
        break;
      case MENU_STATE:
        if (redraw) {
          draw_MenuScreen();
          redraw = false;
        }
        switch (touchedQuadrant) {
          case BTN_NE:
            deviceState = SETTINGS_STATE;
            redraw = true;
            break;
          case BTN_NW:
            deviceState = READY_STATE;
            redraw = true;
            break;
          case BTN_SW:
            deviceState = LOG_STATE;
            redraw = true;
            break;
          case BTN_SE:
            deviceState = SHUTDOWN_STATE;
            redraw = true;
            break;
        }
        break;
      case SETTINGS_STATE:
        if (redraw) {
          draw_SettingsScreen();
          redraw = false;
        }
        if (tempChange) {
          draw_TempButtons();
          tempChange = false;
        }
        switch (touchedQuadrant) {
          case BTN_NE:
            if ( fahrenheit ) {
              fahrenheit = false;
            } else {
              fahrenheit = true;
            }
            tempChange = true;
            break;
          case BTN_NW:
            deviceState = LOG_STATE;
            redraw = true;
            break;
          case BTN_SW:
            deviceState = CALIBRATE_STATE;
            redraw = true;
            break;
          case BTN_SE:
            deviceState = READY_STATE;
            redraw = true;
            break;
        }
        break;
      case CALIBRATE_STATE:
        if (redraw) {
          draw_CalibrateScreen();
          redraw = false;
        }
        if (touchedQuadrant == BTN_NW) {                      // if stop button is pressed, do not save and go back to menu
          deviceState = MENU_STATE;
          redraw = true;
        }
        else if ( touchedQuadrant == BTN_NE) {                // if go button is pressed, calibrate pH sensor
          pH *phSensor = (pH*)sensors.getSensor(PH_SENSOR_ID);
          phSensor->calibrate();
        }
        break;
      case LOG_STATE:
        if (redraw) {
  
          redraw = false;
        }
        if ( touchedQuadrant == BTN_NW) {
          deviceState = MENU_STATE;
          redraw = true;
        }
        else if ( touchedQuadrant == BTN_NE) {
          /*** get the next 5 readings here ****/
          redraw = true;
        }
        break;
      case SHUTDOWN_STATE:      // ******** Need to do this one ******* ///
        if (redraw) {
          draw_ShutdownScreen();
          redraw = false;
        }
        break;
      default:
        break;
    }
}

uint8_t getQuadrantFromPoint(TS_Point *p) {
  if ( p->x > TS_MAXX / 2 && p->y < TS_MAXY / 2) {
    return BTN_NW;
  }
  if ( p->x < TS_MAXX / 2 && p->y < TS_MAXY / 2) {
    return BTN_SW;
  }
  if ( p->x < TS_MAXX / 2 && p->y > TS_MAXY / 2) {
    return BTN_SE;
  }
  if ( p->x > TS_MAXX / 2 && p->y > TS_MAXY / 2) {
    return BTN_NE;
  }

  return BTN_NONE;
}

float cToF(float c) {
  return c * 1.8 + 32;
}

void draw_MainMenu() {
  // Blank screen
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes */
  tft->setTextSize(2);

  /* Draw NW 'Read' button */
  tft->fillRect( 20, 20, 120, 80, ILI9341_GREEN );
  tft->setCursor( 55, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN );
  tft->println("Read");

  /* Draw NE 'Menu' button */
  tft->fillRect(170, 20, 120, 80, ILI9341_YELLOW);
  tft->setCursor(205, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_YELLOW);
  tft->println("Menu");

  /* Draw 'GrowthLine' text logo */
  tft->setTextSize(4);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->setCursor(35, 162);
  tft->println("GrowthLine");
}

void draw_WarmUpScreen() {
  /* Blank screen */
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes and draw text */
  tft->setTextSize(3);
  tft->setCursor( 70, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("Warming up");
}

void draw_SaveScreen() {
  /* Blank screen */
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes and draw text. */
  tft->setTextSize(3);
  tft->setCursor( 62, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("Saving");
}

void draw_ShutdownScreen() {
  /* Blank screen */
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes and draw text. */
  tft->setTextSize(3);
  tft->setCursor( 60, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("Shutting Down");
}

void draw_ReadScreen() {
  // Blank the screen
  tft->fillScreen(ILI9341_BLACK);

  // Set text parameters
  tft->setTextSize(3);

  // Draw NW 'Stop' button
  tft->fillRect( 20, 20, 120, 80, ILI9341_RED);
  tft->setCursor( 45, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED);
  tft->println("Stop");

  // Draw NE 'Save' button
  tft->fillRect(170, 20, 120, 80, ILI9341_GREEN);
  tft->setCursor(195, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN);
  tft->println("Save");
}

void update_Readings() {
  /* Blank out numbers */
  tft->fillRect(140, 101, 180, 139, ILI9341_BLACK);

  /* Set text parameters */
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);

  /* Draw reading 1 - Ambient Light */
  String amb_lite    = "Amb. Lite: ";
  tft->setCursor( 20, 112);
  tft->println(amb_lite + readings.peek().lux);

  /* Draw reading 2 - Air Temperature */
  String air_temp    = "Air Temp.: ";
  tft->setCursor( 20, 132);
  if (fahrenheit) {
    tft->println(air_temp + cToF(readings.peek().airTemperature));
  } else {
    tft->println(air_temp + readings.peek().airTemperature);
  }

  /* Draw reading 3 - Humidity */
  String humidity    = "Humidity : ";
  tft->setCursor( 20, 152);
  tft->println(humidity + readings.peek().humidity);

  /* Draw reading 4 - Soil pH */
  String ph          = "pH       : ";
  tft->setCursor( 20, 172);
  tft->println(ph + readings.peek().pH);

  /* Draw reading 5 - Soil moisture  */
  String moisture    = "Moisture : ";
  tft->setCursor( 20, 192);
  tft->println(moisture + readings.peek().moisture);

  /* Draw reading 6 - Ground temperature */
  String ground_temp = "Gnd. Temp: ";
  tft->setCursor( 20, 212);
  if (fahrenheit) {
    tft->println(air_temp + cToF(readings.peek().groundTemperature));
  } else {
    tft->println(air_temp + readings.peek().groundTemperature);
  }
}

void draw_CalibrateScreen() {
  /* Blank the screen */
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes */
  tft->setTextSize(3);

  /* Draw NW 'Back' button */
  tft->fillRect( 20, 20, 120, 80, ILI9341_RED);
  tft->setCursor( 45, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED);
  tft->println("Back");

  /* Draw NE 'Go' button */
  tft->fillRect(170, 20, 120, 80, ILI9341_GREEN);
  tft->setCursor(215, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN);
  tft->println("Go");

  /* Set text attributes and draw instructions */
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->setCursor( 20, 132); tft->print("Dip the pH probe into a");
  tft->setCursor( 20, 152); tft->print(" 7 pH solution at 25C." );
  tft->setCursor( 20, 172); tft->print(" After 2 min, press Go.");
  tft->setCursor( 20, 212); tft->print(" Press back to cancel." );
}

void draw_MenuScreen() {
  /* Blank screen */
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes */
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_BLACK, ILI9341_YELLOW );

  /* Draw NW 'Back' button */
  tft->fillRect( 20, 20, 120, 80, ILI9341_YELLOW );
  tft->setCursor( 55, 52);
  tft->println("Back");

  /* Draw NE 'Settings' button */
  tft->fillRect(180, 20, 120, 80, ILI9341_YELLOW);
  tft->setCursor(193, 52);
  tft->println("Settings");

  /* Set text color for following buttons */
  tft->setTextColor( ILI9341_BLACK, ILI9341_MAGENTA);

  /* Draw SW 'View Logs' button */
  tft->fillRect( 20, 140, 120, 80, ILI9341_MAGENTA);
  tft->setCursor( 25, 172);
  tft->println("View Logs");

  /* Draw SE 'Shut Down' button */
  tft->fillRect(180, 140, 120, 80, ILI9341_MAGENTA);
  tft->setCursor(185, 172);
  tft->println("Shut Down");
}

void draw_SettingsScreen() {
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_YELLOW );
  tft->fillRect( 20, 140, 120, 80, ILI9341_MAGENTA);
  tft->fillRect(180, 140, 120, 80, ILI9341_MAGENTA);
  tft->setTextSize(2);
  // Write text on button 1
  tft->setCursor( 55, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_YELLOW );
  tft->println("Back");
  draw_TempButtons();
  // Set text color for following buttons
  tft->setTextColor( ILI9341_BLACK, ILI9341_MAGENTA);
  // Write text on button 3
  tft->setCursor( 27, 172);
  tft->println("Calibrate");
  // Write text on button 4
  tft->setCursor(200, 172);
  tft->println("New Log");
}

void draw_TempButtons() {
  // Write text on button 2
  if (fahrenheit) {
    tft->fillRect(180, 20, 60, 80, ILI9341_MAROON);
    tft->setTextColor( ILI9341_BLACK, ILI9341_MAROON);
    tft->setCursor(207, 52);
    tft->print("C");
    tft->fillRect(240, 20, 60, 80, ILI9341_GREEN);
    tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN);
    tft->setCursor(267, 52);
    tft->print("F");
  } else {
    tft->fillRect(180, 20, 60, 80, ILI9341_GREEN);
    tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN);
    tft->setCursor(207, 52);
    tft->print("C");
    tft->fillRect(240, 20, 60, 80, ILI9341_MAROON);
    tft->setTextColor( ILI9341_BLACK, ILI9341_MAROON);
    tft->setCursor(267, 52);
    tft->print("F");
  }
}

void draw_LogScreen(String in_array[]) {
  /* Blank the screen */
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes */
  tft->setTextSize(3);

  /* Draw NW 'Back' button */
  tft->fillRect( 20, 20, 120, 80, ILI9341_RED);
  tft->setCursor( 45, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED);
  tft->println("Back");

  /* Draw NE 'Next' button */
  tft->fillRect(170, 20, 120, 80, ILI9341_GREEN);
  tft->setCursor(195, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN);
  tft->println("Next");

  /* Output lines */
  for (int i = 0; i < 6; i++) {
    tft->setCursor( 20, 82 + (i * 30));
    tft->println(in_array[i]);
  }
}

