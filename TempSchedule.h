#include <Arduino.h>

// Used to store a specific temperature for a desired amount of days.
class TempSchedule {
public:
    TempSchedule(byte temperature, unsigned long days);
    bool isDone();
    void setStartTime(unsigned long startTime);
    byte getTemp();
    void setTemp(byte temperature);
    unsigned long getDuration();
    void setDuration(unsigned long days);

private:
    byte _temperature;
    unsigned long _duration;
    unsigned long _startTime;
    //unsigned long _elapsedTime; TODO: backup time in case of power outages etc.
};

class Schedules {
public:
    static const byte SCHEDULES_MAX_COUNT = 3;
    Schedules();
    bool next();
    void addTempSchedule(byte temperature, unsigned long days);
    byte getSchedulesCount();
    TempSchedule* getTempSchedule(byte index);
    
private:
    byte _schedulesCount;
    TempSchedule* _schedules[SCHEDULES_MAX_COUNT];
};

