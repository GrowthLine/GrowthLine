#ifndef SENSOR_INCLUDE_
#define SENSOR_INCLUDE_

#include <Utilities.h>
#include <Measurements.h>

/* Sensor class, all sensors inherit from this */
class Sensor {
private:
    unsigned int ID;
 public:
    Sensor();
    Sensor( const Sensor &s);
    Sensor( const unsigned int& id );
    ~Sensor();
    unsigned int getId() const;
    virtual List<Read> read() = 0;
    virtual void setUp() = 0;
    virtual void calibrate() = 0;
};

/* Sensors Class */
class Sensors {
    List<Sensor*> sensors;
  public:
    Sensors();
    Sensors(const Sensors &that);

    Sensors& operator=( const Sensors that);

    ~Sensors();

    void addSensor(Sensor* s);

    void setupSensors();

    Sensor* getSensor(unsigned int id);

    List<Sensor*> getSensors() const;

    Reading getReading();
};
#endif /* SENSOR_INCLUDE_ */
