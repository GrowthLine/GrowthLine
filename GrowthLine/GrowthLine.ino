#include "lib.h"

/* Changable Parameters */
const unsigned int READING_FREQUENCY = 1000;     // Length in milliseconds before each reading
const uint8_t NUMBER_OF_READINGS = 5;           // Number of readings we will hold before discarding old one
const int WARMUP_LENGTH = 5000;                 // length of the warm up in milliseconds

/* Variable Declarations */
QueueList<Reading*> readings;                   // List used to hold reading objects. Structured in a queue.
Sensors sensors;                                // Object that will hold and manage all other sensors
int deviceState;                                // State of device
bool redraw;                                    // Flag used to know when to redraw a screen
unsigned long milliseconds;                     // saves milliseconds. Used with millis() to check lenths of time
Adafruit_STMPE610 *ts;                          // pointer to a touch screen object
Adafruit_ILI9341 *tft;                          // pointer to a display object

void setup() {
  Serial.begin(9600);
  readings.setPrinter(Serial);
  deviceState   = READY_STATE;
  redraw        = true;

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
  Serial.println("Setup is complete");
}

/**
   The if (redraw) prevents the entire screen from
   being redrawn every loop() cycle. This prevents
   flickering.
*/
void loop() {
  TS_Point touchedPoint;
  uint8_t touchedQuadrant = BTN_NONE;
  while (ts->touched()) {
    touchedPoint = ts->getPoint();
    if ( touchedPoint.z < 75 )
      touchedQuadrant = getQuadrantFromPoint(&touchedPoint);
  }

  // for debugging purposes
  if (touchedQuadrant != 0)
    Serial.println(touchedQuadrant);

  // State Machine of the device
  switch (deviceState) {
    case READY_STATE:
      if (redraw) {                             // Draws the main menu
        draw_MainMenu();
        redraw = false;
      }
      if ( touchedQuadrant == BTN_NW ) {        // if Read button is pressed, go to Warm-up State
        deviceState = WARMUP_STATE;
        redraw = true;
      }
      else if ( touchedQuadrant == BTN_NE) {    // if Menu button is pressed, go to the meny screen
        deviceState = MENU_STATE;
        redraw = true;
      }
      break;
    case WARMUP_STATE:
      if (redraw) {
        draw_WarmUpScreen();
        redraw = false;
      }
      milliseconds = millis();
      while(millis() - milliseconds < WARMUP_LENGTH)     // Get readings without saving for several seconds without saving to warm up
        sensors.getReading();
      deviceState = READ_STATE
      redraw = true;
      break;
    case READ_STATE:
      if (redraw) {
        draw_ReadScreen();
        redraw = false;
      }
      if ( touchedQuadrant == BTN_NW ) {                 // if stop button is pressed, do not save and go back to main menu
        deviceState = READY_STATE;
        redraw = true;
      }
      else if ( touchedQuadrant == BTN_NE ) {
        deviceState = SAVE_STATE;
        redraw = true;
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
    case MENU_STATE:        // ******** Need to do this one ******* ///
      if (redraw) {
        draw_MenuScreen();
        redraw = false;
      }
      break;
    case CALIBRATE_STATE:     // ******** Need to do this one ******* ///
      if (redraw) {
        draw_CalibrateScreen();
        redraw = false;
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
  if ( p->x > TS_MAXX / 2 && p->y < TS_MAXY / 2)
    return BTN_NW;
  else if ( p->x < TS_MAXX / 2 && p->y < TS_MAXY / 2)
    return BTN_SW;
  else if ( p->x < TS_MAXX / 2 && p->y > TS_MAXY / 2)
    return BTN_SE;
  else if ( p->x > TS_MAXX / 2 && p->y > TS_MAXY / 2)
    return BTN_NE;
  else
    return BTN_NONE;
}

void draw_MainMenu() {
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_GREEN );
  tft->fillRect(170, 20, 120, 80, ILI9341_YELLOW);
  tft->setTextSize(2);
  // Write text on button 1
  tft->setCursor( 55, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN );
  tft->println("Read");
  // Write text on button 2
  tft->setCursor(205, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_YELLOW);
  tft->println("Menu");
  /* Draw 'GrowthLine' */
  tft->setTextSize(4);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->setCursor(35, 162);
  tft->println("GrowthLine");
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
  // Blank the screen
  tft->fillScreen(ILI9341_BLACK);

  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_RED);
  tft->fillRect(170, 20, 120, 80, ILI9341_GREEN);

  // Draw text for 'Stop' button
  tft->setTextSize(3);
  tft->setCursor( 45, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED);
  tft->println("Stop");

  //Draw text for 'Save' button
  tft->setCursor(195, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN);
  tft->println("Save");
}

void update_Readings() {
  // Blank out numbers
  tft->fillRect(140, 101, 180, 139, ILI9341_BLACK);

  // Set text parameters
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);

  // Draw reading 1
  String amb_lite    = "Amb. Lite: ";
  tft->setCursor( 20, 112);
  tft->println(amb_lite + readings.peek()->lux);

  // Draw reading 2
  String air_temp    = "Air Temp.: ";
  tft->setCursor( 20, 132);
  tft->println(air_temp + readings.peek()->airTemperature);

  // Draw reading 3
  String humidity    = "Humidity : ";
  tft->setCursor( 20, 152);
  tft->println(humidity + readings.peek()->humidity);

  // Draw reading 4
  String ph          = "pH       : ";
  tft->setCursor( 20, 172);
  tft->println(ph + readings.peek()->pH);

  // Draw reading 5
  String moisture    = "Moisture : ";
  tft->setCursor( 20, 192);
  tft->println(moisture + readings.peek()->moisture);

  // Draw reading 6
  String ground_temp = "Gnd. Temp: ";
  tft->setCursor( 20, 212);
  tft->println(ground_temp + readings.peek()->groundTemperature);
}

void draw_CalibrateScreen() {

  // Blank the screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_RED);
  tft->fillRect(170, 20, 120, 80, ILI9341_GREEN);
  // Draw text
  tft->setTextSize(3);
  tft->setCursor( 45, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED);
  tft->println("Back");

  tft->setCursor(200, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN);
  tft->println("Go");

  // Draw Instructions
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft->setCursor( 20, 132); tft->print("Dip the pH probe into a");
  tft->setCursor( 20, 152); tft->print("  7 pH solution, then"  );
  tft->setCursor( 20, 172); tft->print("      press Go."        );

  tft->setCursor( 20, 212); tft->print(" Press back to cancel." );
}

void draw_MenuScreen() {
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_YELLOW );
  tft->fillRect(180, 20, 120, 80, ILI9341_MAGENTA);
  tft->fillRect( 20, 140, 120, 80, ILI9341_MAGENTA);
  tft->fillRect(180, 140, 120, 80, ILI9341_MAGENTA);
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
  // Log functionality not yet implemented.
  //tft->setCursor( 55, 172);
  //tft->println("Logs");
  // Write text on button 4
  tft->setCursor(200, 172);
  tft->println("Calibrate");
}

