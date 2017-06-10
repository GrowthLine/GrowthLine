#include "lib.h"

/* Variable Declarations */
List<Reading> readings(5);           // List used to hold reading objects. Structured in a queue.
Sensors sensors;                                // Object that will hold and manage all other sensors
int deviceState;                                // State of device
bool fahrenheit;                                // Saves user information about temperature unit
bool redraw;                                    // Flag used to know when to redraw a screen
bool tempChange;                                // Flag used to ID when to redraw Temp buttons
unsigned long milliseconds;                     // saves milliseconds. Used with millis() to check lenths of time
unsigned int currentRead;                       // holds a value of the latest read displayed on the log screen
String statusBar;                               // holds the message displayed on the status bar
Adafruit_STMPE610 *ts;                          // pointer to a touch screen object
Adafruit_ILI9341 *tft;                          // pointer to a display object
int tempReadingNumber;
Reading tempReading;

void setup() {
  Serial.begin(9600);
  Serial.println("Software Initializing");
  tft = new Adafruit_ILI9341(TFT_CS, TFT_DC);
  tft->begin();
  tft->setRotation(1);
  draw_Startup();
  deviceState   = READY_STATE;
  redraw        = true;
  fahrenheit    = false;
  tempChange    = true;
  currentRead   = 1;
  statusBar     = "GrowthLine";
  String logNumberBuffer = "";                     // holds the settings from the buffer
  String lineReadBuffer = "";

  /* Add the sensors to our Sensors object */
  LightSensor* lightSensor = new LightSensor();
  sensors.addSensor(lightSensor);
  TempHumid* tempHumid = new TempHumid(TEMP_HUMID_PIN);
  sensors.addSensor(tempHumid);
  pH* ph = new pH(PH_RECEIVE_PIN, PH_TRANSMIT_PIN);
  sensors.addSensor(ph);
  TempMoist* tempMoist = new TempMoist(TEMP_MOIST_DATA_PIN, TEMP_MOIST_CLOCK_PIN);
  sensors.addSensor(tempMoist);

  /* Create our touch objects */
  ts = new Adafruit_STMPE610(STMPE_CS);

  /* Setup the sensors */
  sensors.setupSensors();

  /* Setting up the SD card */
  if (! SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
  }
  else {
    Logger::saveEnable = false;
    // If the settings file is missing, make it with the defaults
    if ( !SD.exists("settings.txt")) {
      Serial.println("Making Settings File");
      File settingsFile = SD.open("settings.txt", FILE_WRITE);
      settingsFile.println("TempUnit=C");
      settingsFile.println("LogFile=1");
      settingsFile.println("Reading=1");
      settingsFile.close();
    }
    File settingsFile = SD.open("settings.txt");
    while (settingsFile.available() && (char)settingsFile.read() != '=');               // find the first = sign, which indicates temp. unit
    if (settingsFile.available() && (char)settingsFile.read() == 'F')
      fahrenheit = true;
    while (settingsFile.available() && (char)settingsFile.read() != '=');
    while (settingsFile.available() && (char)settingsFile.peek() != '\n') {
      logNumberBuffer += (char)settingsFile.read();
    }
    while (settingsFile.available() && (char)settingsFile.read() != '=');
    while (settingsFile.available() && (char)settingsFile.peek() != '\n') {
      lineReadBuffer += (char)settingsFile.read();
    }
    if (logNumberBuffer.toInt() != 0) {
      Logger::logFileNumber = logNumberBuffer.toInt();
    }
    else {
      Logger::logFileNumber = 1;
    }
    if (lineReadBuffer.toInt() != 0) {
      Logger::readingNumber = lineReadBuffer.toInt();
    }
    else {
      Logger::readingNumber = 1;
    }
    settingsFile.close();
    Logger::checkLogExists();
  }
  /* check that touch screen is started propperly */
  while (true) {
    if (ts->begin()) {
      break;
    } else {
      Serial.println("Touchscreen controller start failure!");
    }
  }
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
        Serial.println("device is ready");
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
      for ( int i = 0; i < 5; i++ ) {                       // get initial 5 readings
        readings.add( sensors.getReading() );
        while (millis() - milliseconds < READING_FREQUENCY);
        milliseconds = millis();
      }

      while (!stableReadings(readings)) {
        milliseconds = millis();
        while ( millis() - milliseconds < READING_FREQUENCY);
        readings.add( sensors.getReading() );
      }
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
          if (Logger::saveEnable) {             // if save button is pressed, save and go back to main menu
            deviceState = SAVE_STATE;
            redraw = true;
          }
          break;
        case BTN_NW:                   // if stop button is pressed, do not save and go back to main menu
          deviceState = READY_STATE;
          redraw = true;
          statusBar = "GrowthLine";
          readings.clear();
          break;
      }
      if ( millis() - milliseconds > READING_FREQUENCY) {   // updates displayed reading
        readings.add( sensors.getReading() );
        milliseconds = millis();
        update_Readings();
      }
      break;
    case SAVE_STATE:
      if (redraw) {
        draw_SaveScreen();
        redraw = false;
      }
      Serial.println("Saving log file");
      tempReading = readings.get_new();
      Logger::saveLog(tempReading, fahrenheit);
      Serial.println("Log saved");
      deviceState = READY_STATE;
      redraw = true;
      statusBar = "Read Saved";
      readings.clear();
      Logger::saveSettings(fahrenheit);
      Serial.println("Entering READY_STATE");
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
          statusBar = "GrowthLine";
          break;
        case BTN_SW:
          if ( Logger::saveEnable ) {
            deviceState = LOG_STATE;
            redraw = true;
          }
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
          Logger::saveSettings(fahrenheit);
          tempChange = true;
          break;
        case BTN_NW:
          deviceState = MENU_STATE;
          redraw = true;
          break;
        case BTN_SW:
          deviceState = CALIBRATE_STATE;
          redraw = true;
          break;
        case BTN_SE:
          if (Logger::saveEnable) {
            Logger::logFileNumber += 1;
            Logger::checkLogExists();
            Logger::readingNumber = 1;
            Logger::saveSettings(fahrenheit);
            deviceState = READY_STATE;
            redraw = true;
            statusBar = "NewLogFile";
          }
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
        pH* phSensor = (pH*)sensors.getSensor(PH_SENSOR_ID);
        phSensor->calibrate();
        deviceState = READY_STATE;
        redraw = true;
        statusBar = "Calibrated";
      }
      break;
    case LOG_STATE:
      if (redraw) {
        String logs[5] = {"", "", "", "", ""};
        Logger::getLogs(String("log") + Logger::logFileNumber + ".txt", logs, &currentRead);
        draw_LogScreen(logs);
        redraw = false;
      }
      if ( touchedQuadrant == BTN_NW) {
        deviceState = MENU_STATE;
        currentRead = 1;
        redraw = true;
      }
      else if ( touchedQuadrant == BTN_NE) {
        String logs[5] = {"", "", "", "", ""};
        Logger::getLogs(String("log") + Logger::logFileNumber + ".txt", logs, &currentRead);
        update_Logs(logs);
        if (currentRead >= Logger::readingNumber)
          currentRead = 1;
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

/* Display Functions */
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
  tft->println(statusBar);
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
  tft->setCursor( 82, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("Saving");
}

void draw_Startup() {
  /* Blank screen */
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes and draw text. */
  tft->setTextSize(3);
  tft->setCursor( 30, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("  Initializing  ");
}

void draw_ShutdownScreen() {
  /* Blank screen */
  tft->fillScreen(ILI9341_BLACK);

  /* Set text attributes and draw text. */
  tft->setTextSize(3);
  tft->setCursor( 30, 100);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->println("Safe to Shutdown");
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
  tft->println(amb_lite + readings.get_new().getReadValueByType(ReadType::LUX));

  /* Draw reading 2 - Air Temperature */
  String air_temp    = "Air Temp.: ";
  tft->setCursor( 20, 132);
  if (fahrenheit) {
    tft->println(air_temp + cToF(readings.get_new().getReadValueByType(ReadType::AIR_TEMP)) + "F");
  } else {
    tft->println(air_temp + readings.get_new().getReadValueByType(ReadType::AIR_TEMP) + "C");
  }

  /* Draw reading 3 - Humidity */
  String humidity    = "Humidity : ";
  tft->setCursor( 20, 152);
  tft->println(humidity + readings.get_new().getReadValueByType(ReadType::HUMIDITY));

  /* Draw reading 4 - Soil pH */
  switch (phStatus(readings.get_new().getReadValueByType(ReadType::PH))) {
    case BAD_LOW:
      tft->setTextColor( ILI9341_BLUE, ILI9341_BLACK);
      break;
    case GOOD:
      tft->setTextColor( ILI9341_GREEN, ILI9341_BLACK);
      break;
    case BAD_HIGH:
      tft->setTextColor( ILI9341_RED, ILI9341_BLACK);
      break;
    default:
      break;
  }
  String ph          = "pH       : ";
  tft->setCursor( 20, 172);
  tft->println(ph + readings.get_new().getReadValueByType(ReadType::PH));

  /* Draw reading 5 - Soil moisture  */
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  String moisture    = "Moisture : ";
  tft->setCursor( 20, 192);
  tft->println(moisture + readings.get_new().getReadValueByType(ReadType::MOISTURE));

  /* Draw rea
    ding 6 - Ground temperature */
  switch (groundTempStatus(readings.get_new().getReadValueByType(ReadType::GROUND_TEMP))) {
    case BAD_LOW:
      tft->setTextColor( ILI9341_BLUE, ILI9341_BLACK);
      break;
    case GOOD:
      tft->setTextColor( ILI9341_GREEN, ILI9341_BLACK);
      break;
    case BAD_HIGH:
      tft->setTextColor( ILI9341_RED, ILI9341_BLACK);
      break;
    default:
      break;
  }
  String ground_temp = "Gnd. Temp: ";
  tft->setCursor( 20, 212);
  if (fahrenheit) {
    tft->println(ground_temp + cToF(readings.get_new().getReadValueByType(ReadType::GROUND_TEMP)) + "F");
  } else {
    tft->println(ground_temp + readings.get_new().getReadValueByType(ReadType::GROUND_TEMP) + "C");
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

  /* Output headers */
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->setCursor( 20, 122);
  tft->println("# gT Moi pH aT Hum Lux");

  /* Output lines */
  for (int i = 0; i < 5; i++) {
    tft->setCursor( 20, 142 + (i * 20));
    tft->println(in_array[i]);
  }
}

void update_Logs(String in_array[]) {
  /* Blank out logs */
  tft->fillRect(10, 142, 300, 100, ILI9341_BLACK);

  /* Output lines */
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  for (int i = 0; i < 5; i++) {
    tft->setCursor( 20, 142 + (i * 20));
    tft->println(in_array[i]);
  }
}
