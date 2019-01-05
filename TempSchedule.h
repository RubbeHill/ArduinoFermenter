#include <Arduino.h>

// Used to store a specific temperature for a desired amount of days.
class TempSchedule {
public:
    TempSchedule(byte temperature, unsigned long days);
    unsigned long getStart();
    bool isDone();
    void setStartTime(unsigned long startTime);

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
    bool addTempSchedule(byte temperature, unsigned long days);
    byte _schedulesCount;
    
private:
    byte _currentSchedule;

    TempSchedule* _schedules[SCHEDULES_MAX_COUNT];
};

