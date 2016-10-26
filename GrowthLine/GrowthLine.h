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

class Reading {
  public:
    float lux;
    float airTemperature;
    float humidity;
    float pH;
    float groundTemperature;
    float moisture;

    Reading( float l, float aT, float h, float ph, float gT, float m );
    Reading();
    
  private:
};

