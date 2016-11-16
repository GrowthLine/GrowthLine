#include "lib.h"


/* Variable Declarations */
QueueList<Reading*> readings;
Sensors sensors;
const uint8_t NUMBER_OF_READINGS = 5;           // Number of readings we will hold before discarding old one
int deviceState, button;                        // State of device
bool redraw, have_readings;
const int totalSensors = 4;

Adafruit_STMPE610 *ts;
Adafruit_ILI9341 *tft;

void setup() {
  Serial.begin(9600);
  readings.setPrinter(Serial);
  deviceState   = READY_STATE;
  button        = BTN_NONE;
  have_readings = false;

  /* Add the sensors to our Sensors object */
  sensors.addSensor(new LightSensor());
  sensors.addSensor(new TempHumid(TEMP_HUMID_PIN));
  sensors.addSensor(new pH(PH_RECEIVE_PIN, PH_TRANSMIT_PIN));
  sensors.addSensor(new TempMoist(TEMP_MOIST_DATA_PIN, TEMP_MOIST_CLOCK_PIN));

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
  redraw = true;
  Serial.println("Setup is complete");
}
/**
   The if (redraw) prevents the entire screen from
   being redrawn every loop() cycle. This prevents
   flickering.
*/
void loop() {
  switch (deviceState) {
    case READY_STATE:
      if (redraw) {
        draw_MainMenu();
        redraw = false;
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
      if (readings.count() > NUMBER_OF_READINGS)
        readings.pop();
      readings.push( sensors.getReading() );
      if (redraw) {
        draw_ReadScreen();
        redraw = false;
      } else {
        update_Readings();
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
      if (redraw) {
        draw_CalibrateScreen();
        redraw = false;
      }
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
  delay(1000);
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
void update_Readings() {
  tft->fillRect(  0, 101, 320, 240, ILI9341_BLACK);
  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  // Draw reading 1
  tft->setCursor( 20, 112);
  tft->println("Amb. Lite: ");
  //tft->println("Amb. Lite: " + readings.peek()->lux);

  // Draw reading 2
  tft->setCursor( 20, 132);
  tft->println("Air Temp.: ");
  //tft->println("Air Temp.: " + readings.peek()->airTemperature);

  // Draw reading 3
  tft->setCursor( 20, 152);
  tft->println("Humidity : ");
  //tft->println("Humidity : " + readings.peek()->humidity);

  // Draw reading 4
  tft->setCursor( 20, 172);
  tft->println("pH       : ");
  //tft->println("pH       : " + readings.peek()->pH);

  // Draw reading 5
  tft->setCursor( 20, 192);
  tft->println("Moisture : ");
  //tft->println("Moisture : " + readings.peek()->moisture);

  // Draw reading 6
  tft->setCursor( 20, 212);
  tft->println("Gnd. Temp: ");
  //tft->println("Gnd. Temp: " + readings.peek()->groundTemperature);
}

void draw_ReadScreen() {

  // Blank the screen
  tft->fillScreen(ILI9341_BLACK);
  // Draw boxes
  tft->fillRect( 20, 20, 120, 80, ILI9341_RED);
  tft->fillRect(170, 20, 120, 80, ILI9341_GREEN);
  // Draw text
  tft->setTextSize(3);
  tft->setCursor( 45, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_RED);
  tft->println("Stop");

  tft->setCursor(195, 52);
  tft->setTextColor( ILI9341_BLACK, ILI9341_GREEN);
  tft->println("Save");

  tft->setTextSize(2);
  tft->setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  // Draw reading 1
  tft->setCursor( 20, 112);
  tft->print("Amb. Lite: ");
  //tft->println(readings.peek()->lux);

  // Draw reading 2
  tft->setCursor( 20, 132);
  //tft->print("Air Temp.: ");
  tft->println(readings.peek()->airTemperature);

  // Draw reading 3
  tft->setCursor( 20, 152);
  //tft->print("Humidity : ");
  tft->println(readings.peek()->humidity);

  // Draw reading 4
  tft->setCursor( 20, 172);
  //tft->print("pH       : ");
  tft->println(readings.peek()->pH);

  // Draw reading 5
  tft->setCursor( 20, 192);
  //tft->print("Moisture : ");
  tft->println(readings.peek()->moisture);

  // Draw reading 6
  tft->setCursor( 20, 212);
  //tft->print("Gnd. Temp: ");
  tft->println(readings.peek()->groundTemperature);
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
  tft->setCursor( 55, 172);
  tft->println("Logs");
  // Write text on button 4
  tft->setCursor(200, 172);
  tft->println("Calibrate");
}

