#ifndef LOGGER_INCLUDE_
#define LOGGER_INCLUDE_

#include <Measurements.h>
#include <WString.h>

/**
 * Logger static class that handles reading and saving from SD card
 */
class Logger {
private:
    Logger();
public:
    static bool saveEnable;
    static unsigned long logFileNumber;
    static unsigned long readingNumber;

    /* Overwrite "settings.txt" when changes are made */
    static void saveSettings (bool fahrenheit);

    /* Save log to SD card */
    static void saveLog(Reading& reading, const bool& fahrenheit);

    /* Check if log file exists and create if it does not */
    static void checkLogExists();

    /* Reads 5 log files starting from the logNumber */
    static void getLogs(const String& fileName, String logs[], unsigned int *logNumber );
};

#endif /* LOGGER_INCLUDE_ */