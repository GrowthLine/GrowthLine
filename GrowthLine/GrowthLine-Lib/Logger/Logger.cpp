#include "Logger.h"
#include <SD.h>

Logger::Logger() {}

bool Logger::saveEnable = false;
unsigned long Logger::logFileNumber = 1;
unsigned long Logger::readingNumber = 1;

void Logger::saveSettings(bool fahrenheit) {
    if (saveEnable) {
        SD.remove("settings.txt");
        File settingsFile = SD.open("settings.txt", FILE_WRITE);
        settingsFile.print("TempUnit");
        settingsFile.println(fahrenheit ? "F" : "C" );
        settingsFile.println("LogFile=" + String(logFileNumber));
        settingsFile.println("Reading=" + String(readingNumber));
        settingsFile.close();
        Serial.println("Settings Updated!");
    } else {
        Serial.println("Save was not enabled during setup");
    }
}

void Logger::saveLog(Reading& reading, const bool& fahrenheit) {
    String logFileName = "log" + String(logFileNumber) + ".txt";
    if (SD.exists(logFileName)) {
        File logFile = SD.open(logFileName, FILE_WRITE);
        logFile.println(String(readingNumber) + "," + reading.toStringSpecific(fahrenheit));
        logFile.close();
        readingNumber += 1;
    }
    else {
        Serial.println("Log File Not Found!");
    }
}

void Logger::checkLogExists()  {
    String logFileName = "log" + String(logFileNumber) + ".txt";
    String fileHeader = "Reading Number,Soil Temperature,Soil Moisture,Soil pH,Air Temperature,Air Humidity,Lux";
    if (!SD.exists(logFileName)) {
        File logFile = SD.open(logFileName, FILE_WRITE);
        logFile.println(fileHeader);
        logFile.close();
        Serial.println("New log file created " + logFileName);
    }
    else {
        Serial.println("Log File Found!");
    }
}

void Logger::getLogs(const String& fileName, String logs[], unsigned int *logNumber) {
    File logFile = SD.open(fileName);

    // Read data already displayed in the log scereen
    uint8_t logCounter = 0;
    while (logCounter != *logNumber && logFile.available() ) {
        if (logFile.read() == '\n')
        logCounter += 1 ;
    }

    if(!logFile.available()) {                             // if the file has been fully read, just exit
        logFile.close();
        return;
    }

    uint8_t index = 0;                                  // current array index
    bool use = true;                                    // used to tell whether we save character or not
    uint8_t commaCount = 0;                             // used to tell when we arrive to pH field
    while ( logFile.available() ) {
        if ( index == 5 )                                 // once we have 5 logs, exit
        break;
        char current = logFile.read();
        if ( current == '\n') {                           // we finished reading one record
        index += 1;
        commaCount = 0;
        use = true;
        continue;
        }
        if ( current == '.' && commaCount != 3) {         // truncate fractinoal part, except for pH
        use = false;
        continue;
        }
        else if ( current == ',') {                       // start saving characters again, for next fields
        commaCount += 1;
        use = true;
        }
        if ( use )                                        // concatinate the current character to the current log
        logs[index] += current;
    }
    logFile.close();
    *logNumber += index;
}