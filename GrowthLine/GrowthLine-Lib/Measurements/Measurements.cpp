#include "Measurements.h"

Read::Read() : Read(0, ReadType::UNKNOWN) {}

Read::Read(const Read &that) : Read(that.value, that.type) {}

Read::Read(const float &v, const ReadType &t) : value(v), type(t) {}

Read::~Read() {}

Read& Read::operator=(const Read that) {
    if (this != &that) {
        value = that.value;
        type = that.type;
    }
    return *this;
}

void Read::setValue(const float &v) { value = v; }
float Read::getValue() const { return value; }
void Read::setType(const ReadType &t) { type = t; }
ReadType Read::getType() const { return type; }

String Read::getTypeName() const {
    switch(type) {
        case PH:
            return "PH";
        case GROUND_TEMP:
            return "Ground Temp";
        case MOISTURE:
            return "Moisture";
        case LUX:
            return "Lux";
        case AIR_TEMP:
            return "Air Temp";
        case HUMIDITY:
            return "Humidity";
        default:
            return "Unknown";
    }
}

String Read::toString() const {
    return (String)"" + getTypeName() + ":" + value;
}

Reading::Reading() : readingId(-1) {}

Reading::Reading(int &r) : readingId(r) {}

Reading::Reading(const Reading &that) : reads(that.reads), readingId(that.readingId) {}

Reading::~Reading() {}

Reading& Reading::operator=(const Reading that) {
    if (this != &that) {
        reads = that.reads;
        readingId = that.readingId;
    } 
    return *this;
}

int Reading::getReadingId() const { return readingId; }
void Reading::addReads(const Read &r) { reads.add(r); }
void Reading::addReads(List<Read>& rds) {
    for (rds.begin(); rds.isNotAtEnd(); rds.advance()) {
        Read read = rds.getAtCursor();
        reads.add(read);
    }
}

Read Reading::getReadByType(ReadType t) {
    Read fRead(-1, ReadType::ERROR);
    for (reads.begin(); reads.isNotAtEnd(); reads.advance()) {
        Read currentRead = reads.getAtCursor();
        if (currentRead.getType() == t) {
            fRead = currentRead;
        }
    }
    return fRead;
}

float Reading::getReadValueByType(ReadType t) { return getReadByType(t).getValue(); }
List<Read> Reading::getReads() const { return reads; }
String Reading::toString() {
    if (!reads.isEmpty()) {
        String readingString = "";
        reads.begin();
        Read currentRead = reads.getAtCursor();
        readingString += currentRead.toString();
        reads.advance();
        for (; reads.isNotAtEnd(); reads.advance()) {
            currentRead = reads.getAtCursor();
            readingString += "," + currentRead.toString();
        }
        return readingString;
    } else {
        return (String)"Empty Reading";
    }
}

String Reading::toStringSpecific(const bool &fah) {
    if (!reads.isEmpty()) {
        String readingString = "";
        reads.begin();
        Read currentRead = reads.getAtCursor();
        if (currentRead.getType() == ReadType::AIR_TEMP || currentRead.getType() == ReadType::GROUND_TEMP) {
            if (fah) {
                readingString += currentRead.getTypeName() + ":" + cToF(currentRead.getValue()) + "F";
            } else {
                readingString += currentRead.getTypeName() + ":" + cToF(currentRead.getValue()) + "C";    
            }
        } else {
            readingString += currentRead.toString();
        }
        reads.advance();

        for (; reads.isNotAtEnd(); reads.advance()) {
            currentRead = reads.getAtCursor();
            if (currentRead.getType() == ReadType::AIR_TEMP || currentRead.getType() == ReadType::GROUND_TEMP) {
                if (fah) {
                readingString += currentRead.getTypeName() + ":" + cToF(currentRead.getValue()) + "F";
                } else {
                    readingString += currentRead.getTypeName() + ":" + cToF(currentRead.getValue()) + "C";    
                }
            } else {
                readingString += "," + currentRead.toString();
            }
        }
        return readingString;
    } else {
        return (String)"Empty Reading";
    }
}
