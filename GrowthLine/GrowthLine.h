#define READY_STATE 0
#define WARMUP_STATE 1
#define READ_STATE 2
#define SAVE_STATE 3
#define MENU_STATE 4
#define CALIBRATE_STATE 5
#define SHUTDOWN_STATE 6


#define phReceivePin 2
#define phTransmitPin 3
#define tempHumidPin 5
// For the touch controller
#define STMPE_CS 8
// For the TFT screen
#define TFT_DC 9
#define TFT_CS 10

// Touch screen min/max co-ordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// Button constants
#define BTN_NONE 0
#define BTN_NW   1
#define BTN_NE   2
#define BTN_SW   3
#define BTN_SE   4
#define BTN_TOPLEFT   1
#define BTN_TOPRIGHT  2
#define BTN_LIST_1    3
#define BTN_LIST_2    4
#define BTN_LIST_3    5
