#include "Sensor.h"

Sensor::Sensor() {}

Sensor::Sensor(const Sensor &s) : ID(s.ID) {}

Sensor::Sensor(const unsigned int& id) : ID(id) {}

Sensor::~Sensor() {}

unsigned int Sensor::getId() const { return ID; }

Sensors::Sensors() {}

Sensors::Sensors(const Sensors &that) : sensors(that.sensors) {}

Sensors& Sensors::operator=(const Sensors that) {
    if (this != &that) {
        sensors = that.sensors;
    }
    return *this;
}

Sensors::~Sensors() {}

void Sensors::addSensor(Sensor* s) {
    sensors.add(s);
}

void Sensors::setupSensors() {
    for (sensors.begin(); sensors.isNotAtEnd(); sensors.advance()) {
        Sensor* sensor = sensors.getAtCursor();
        sensor->setUp();
    }
}

Sensor* Sensors::getSensor(unsigned int id) {
    for (sensors.begin(); sensors.isNotAtEnd(); sensors.advance()) {
        Sensor* sensor = sensors.getAtCursor();
        if ( sensor->getId() == id ) {
        return sensor;
        }
    }
    return nullptr;
}

List<Sensor*> Sensors::getSensors() const {
    return sensors;
}

Reading Sensors::getReading() {
    Reading reading;
    for (sensors.begin(); sensors.isNotAtEnd(); sensors.advance() ) {
        Sensor* sensor = sensors.getAtCursor();
        List<Read> reads = sensor->read();
        reading.addReads(reads);
    }
    return reading;
}