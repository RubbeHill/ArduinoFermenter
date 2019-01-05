#include "TempSchedule.h"

TempSchedule::TempSchedule(byte temperature, unsigned long days)
: _temperature(temperature),
  _duration(days * 86400000), // Convert days to milliseconds
  _startTime(millis()) {
}

unsigned long TempSchedule::getStart() {
    return _startTime;
}

bool TempSchedule::isDone() {
    return millis() - _startTime >= _duration;
}

void TempSchedule::setStartTime(unsigned long startTime) {
    _startTime = startTime;
}

Schedules::Schedules() 
: _currentSchedule(0),
  _schedulesCount(0) {
}

bool Schedules::next() {
    if(_schedulesCount > 0) {
        if (_schedules[_currentSchedule]->isDone()) {
            _currentSchedule = (_currentSchedule + 1) % 3;
            _schedules[_currentSchedule]->setStartTime(millis());
            return true;
        }
    }
    return false;
}

bool Schedules::addTempSchedule(byte temperature, unsigned long days) {
    if (_schedulesCount < SCHEDULES_MAX_COUNT) {
        TempSchedule* newSchedule = new TempSchedule(temperature, days);
        _schedulesCount++;
        _schedules[(_currentSchedule + _schedulesCount - 1) % 3] = newSchedule; // Add into next free space in the array (using overflow)
        Serial.println("Added ts");
        return true;
    }
    Serial.println("Failed ts");
    return false;
}

