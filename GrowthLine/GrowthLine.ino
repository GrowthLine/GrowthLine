#include "lib.h"

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
unsigned int currentRead;                       // holds a value of the latest read displayed on the log screen
String statusBar;                               // holds the message displayed on the status bar
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
  currentRead   = 1;
  statusBar     = "GrowthLine";
  String logNumberBuffer = "";                     // holds the settings from the buffer
  String lineReadBuffer = "";

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
  // If the settings file is missing, make it with the defaults
  if (! SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    saveEnable = false;
  }
  else {
    if ( !SD.exists("settings.txt")) {
      File settingsFile = SD.open("settings.txt");
      settingsFile.println("TempUnit=C");
      settingsFile.println("LogFile=1");
      settingsFile.println("Reading=1");
      settingsFile.close();
    }
    else {
      File settingsFile = SD.open("settings.txt");
      while (settingsFile.read() != '=');               // find the first = sign, which indicates temp. unit
      if (settingsFile.read() == 'F')
        fahrenheit = true;
      while (settingsFile.read() == '=') {
        while (settingsFile.peek() != '\n') {
          logNumberBuffer += settingsFile.read();
        }
      }
      while (settingsFile.read() == '=') {
        while (settingsFile.peek() != '\n') {
          lineReadBuffer += settingsFile.read();
        }
      }
      if (logNumberBuffer.toInt() != 0) {
        logFileNumber = logNumberBuffer.toInt();
      }
      else {
        logFileNumber = 1;
      }
      if (lineReadBuffer.toInt() != 0) {
        readingNumber = lineReadBuffer.toInt();
      }
      else {
        readingNumber = 1;
      }

    }

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
      for ( int i = 0; i < 5; i++ ) {                       // get initial 5 readings
        readings.push( sensors.getReading() );
        while (millis() - milliseconds < READING_FREQUENCY);
        milliseconds = millis();
      }

      while (!stableReadings(&readings)) {
        milliseconds = millis();
        while ( millis() - milliseconds < READING_FREQUENCY);
        if (readings.count() == NUMBER_OF_READINGS)
          readings.pop();
        readings.push( sensors.getReading() );
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
          if (saveEnable) {             // if save button is pressed, save and go back to main menu
            deviceState = SAVE_STATE;
            redraw = true;
          }
          break;
        case BTN_NW:                   // if stop button is pressed, do not save and go back to main menu
          deviceState = READY_STATE;
          redraw = true;
          statusBar = "GrowthLine";
          while (!readings.isEmpty()) // Clear the readings list
            readings.pop();
          break;
      }
      if ( millis() - milliseconds > READING_FREQUENCY) {   // updates displayed reading
        update_Readings();
        if (readings.count() == NUMBER_OF_READINGS)
          readings.pop();
        readings.push( sensors.getReading() );
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
      statusBar = "Read Saved";
      while (!readings.isEmpty())
        readings.pop();
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
          if ( saveEnable ) {
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
          if (saveEnable) {
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
        pH *phSensor = (pH*)sensors.getSensor(PH_SENSOR_ID);
        phSensor->calibrate();
        deviceState = READY_STATE;
        redraw = true;
        statusBar = "Calibrated";
      }
      break;
    case LOG_STATE:
      if (redraw) {
        String fileName = "log";
        File logFile = SD.open(fileName + logFileNumber + ".txt");
        String logs[5] = {"", "", "", "", ""};
        String currentLog = "";
        uint8_t counter = 0;
        Serial.println("Entering Loop");
        bool use = true;
        uint8_t commaCount = 0;
        while ( logFile.available() ) {
          if ( counter == 5 )
            break;
          char current = logFile.read();
          if ( current == '\n') {
            counter += 1;
            commaCount = 0;
            use = true;
            continue;
          }
          if ( current == '.' && commaCount != 3) {
            use = false;
            continue;
          }
          else if ( current == ',') {
            commaCount += 1;
            use = true;
          }
          if ( use )
            logs[counter] += current;
        }
        logFile.close();
        Serial.println("Closing file and drawing");
        draw_LogScreen(logs);
        Serial.println("Drawing complete");
        currentRead = counter;
        redraw = false;
      }
      if ( touchedQuadrant == BTN_NW) {
        deviceState = MENU_STATE;
        currentRead = 0;
        redraw = true;
      }
      else if ( touchedQuadrant == BTN_NE) {
        Serial.println("Printing the next logs");
        String fileName = "log";
        String logs[5] = {"", "", "", "", ""};
        File logFile = SD.open(fileName + logFileNumber + ".txt");
        String currentLog = "";
        uint8_t logCounter = 0;
        while (logCounter != currentRead && logFile.available() ) {
          if (logFile.read() == '\n')
            logCounter += 1 ;
        }
        uint8_t counter = 0;
        Serial.println("Entering Loop");
        bool use = true;
        uint8_t commaCount = 0;
        while ( logFile.available() ) {
          if ( counter == 5 )
            break;
          char current = logFile.read();
          Serial.print(current);
          if ( current == '\n') {
            counter += 1;
            commaCount = 0;
            use = true;
            continue;
          }
          if ( current == '.' && commaCount != 3) {
            use = false;
            continue;
          }
          else if ( current == ',') {
            commaCount += 1;
            use = true;
          }
          if ( use )
            logs[counter] += current;
        }
        logFile.close();
        Serial.println("updating logs");
        update_Logs(logs);
        Serial.println("Logs Done");
        currentRead += counter;
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
    tft->println(air_temp + cToF(readings.peek().airTemperature) + "F");
  } else {
    tft->println(air_temp + readings.peek().airTemperature + "C");
  }

  /* Draw reading 3 - Humidity */
  String humidity    = "Humidity : ";
  tft->setCursor( 20, 152);
  tft->println(humidity + readings.peek().humidity);

  /* Draw reading 4 - Soil pH */
  switch (phStatus(readings.peek().pH)) {
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
  tft->println(ph + readings.peek().pH);

  /* Draw reading 5 - Soil moisture  */
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  String moisture    = "Moisture : ";
  tft->setCursor( 20, 192);
  tft->println(moisture + readings.peek().moisture);

  /* Draw rea
    ding 6 - Ground temperature */
  switch (groundTempStatus(readings.peek().groundTemperature)) {
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
    tft->println(air_temp + cToF(readings.peek().groundTemperature) + "F");
  } else {
    tft->println(air_temp + readings.peek().groundTemperature + "C");
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
  tft->fillRect(20, 142, 280, 100, ILI9341_BLACK);

  /* Output lines */
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  for (int i = 0; i < 5; i++) {
    tft->setCursor( 20, 142 + (i * 20));
    tft->println(in_array[i]);
    Serial.println(in_array[i]);
  }
}

//Overwrite "settings.txt" when changes are made
void newSettings (bool saveEnable){
  if (saveEnable){
      SD.remove("settings.txt");
      File settingsFile = SD.open("settings.txt");
      settingsFile.println("TempUnit=" + fahrenheit? "F" : "C" );
      settingsFile.println("LogFile=" + String(logFileNumber));
      settingsFile.println("Reading=" + String(readingNumber));
      settingsFile.close();
  }
}

//Save log to SD card
void saveLog(unsigned int logFileNumber, unsigned int readingNumber, QueueList<Reading> readings){
  String logFileName = "log" + String(logFileNumber) + ".txt";
  if (SD.exists(logFileName)) {
    File logFile = SD.open(logFileName, FILE_WRITE);
    logFile.println(String(readingNumber) + "," + readings.peek().toString(fahrenheit));
    logFile.close();
  }
  else {
    Serial.println("Log File Not Found!");
  }
}

//Check if log file exists and create it if it does not.
void checkLogExists(unsigned int logFileNumber){
  String logFileName = "log" + String(logFileNumber) + ".txt";
  String fileHeader = "Reading Number,Soil Temperature,Soil Moisture,Soil pH,Air Temperature,Air Humidity,Lux";
  if (!SD.exists(logFileName)) {
    File logFile = SD.open(logFileName, FILE_WRITE);
    logFile.println(fileHeader);
    logFile.close();
  }
  else {
    Serial.println("Log File Found!");
  }
}

