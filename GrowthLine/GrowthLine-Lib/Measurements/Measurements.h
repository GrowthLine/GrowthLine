#ifndef MEASUREMENTS_INCLUDE_
#define MEASUREMENTS_INCLUDE_

#include <Utilities.h>
#include <WString.h>

enum ReadType {
    UNKNOWN,
    PH,
    GROUND_TEMP,
    MOISTURE,
    LUX,
    AIR_TEMP,
    HUMIDITY,
    ERROR
};

class Read {
private:
    float value;
    ReadType type;
public:

    // default constructor
    Read();

    // Copy constructor
    Read(const Read &that);

    // full constructor
    Read(const float &v, const ReadType &t);

    // destructor
    ~Read();

    // assignment operator
    Read& operator=( const Read that);

    // setter for read value
    void setValue( const float &v );

    // getter for read value
    float getValue() const;

    // setter for read type
    void setType( const ReadType &t );

    // getter for read type
    ReadType getType() const;

    // gets the name of the read type
    String getTypeName() const;

    // string representation of a Read
    String toString() const;
};

/* Reading class holds logic for a single reading. A single reading
 * contains any number of reads
 */
class Reading {
  protected:
    List<Read> reads;
    int readingId;
  
  public:
    // default constructor
    Reading();

    // Copnstructor providing id only
    Reading(int &r);

    // Copy constructor for Reading object
    Reading(const Reading &that);

    // destructor
    ~Reading();

    // Assignment operator
    Reading& operator=( const Reading that);

    // getter for id
    int getReadingId() const;

    // Adds a read to the reading set
    void addReads(const Read &r);

     // Adds list of reads to the reading set
    void addReads(List<Read>& rds);

    /**
     * Finds a read that has the given ReadType. If no read is found, a read of type
     * ERROR and value of -1 is returned instead.
     *
     * @param t - the ReadType
     * @return - the found read or unknown read if not found
     */
    Read getReadByType(ReadType t);

    /**
     * gives the value to the read that has the given ReadType. If no read is found,
     * a read value of 0 is returned instead.
     *
     * @param t - the ReadType
     * @return - the value of the found read or 0 if not found
     */
    float getReadValueByType(ReadType t);

    // getter for the reads
    List<Read> getReads() const;

    /**
     * Returns string representation of this Reading
     *
     * @return - string representation of the reading
     */
    String toString();

    /**
     * Returns string representation of this Reading with specific data given (currently temp unit)
     *
     * @param fah - whether wanted temperature shall be reported as Fahrenheit or not
     * @return - string representation of the reading
     */
    String toStringSpecific(const bool &fah);
};
#endif /* MEASUREMENTS_INCLUDE_ */
