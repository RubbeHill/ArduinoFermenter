#include "TempSchedule.h"

TempSchedule::TempSchedule(int temperature, unsigned long days)
: _temperature(temperature),
  _duration(days * 10000),//86400000), // Convert days to milliseconds
  _startTime(millis()) {
}

bool TempSchedule::isDone() {
    return millis() - _startTime >= _duration;
}

void TempSchedule::setStartTime(unsigned long startTime) {
    _startTime = startTime;
}

int TempSchedule::getTemp() {
    return _temperature;
}

void TempSchedule::setTemp(int temperature) {
    _temperature = temperature;
}

unsigned long TempSchedule::getDuration() {
    return _duration;
}

void TempSchedule::setDuration(unsigned long days) {
    _duration = days * 10000; //86400000;
}

Schedules::Schedules() 
: _schedulesCount(0) {
}

bool Schedules::next() {
    if (_schedulesCount > 0) {
        if (_currentSchedule == NULL) {
            _currentSchedule = _schedules[0];
            _currentSchedule->setStartTime(millis());
            return true;
        } else if (_currentSchedule->isDone()) {
            delete _currentSchedule;
            for (int i = 0; i < _schedulesCount; i++) {
                _schedules[i] = _schedules[i+1];
            }
            _schedulesCount--;
            _currentSchedule = NULL;
            return this->next();
        }
    }
    return false;
}

void Schedules::addTempSchedule(int temperature, unsigned long days) {
    if (_schedulesCount < SCHEDULES_MAX_COUNT) {
        TempSchedule* newSchedule = new TempSchedule(temperature, days);
        _schedules[_schedulesCount] = newSchedule; 
        _schedulesCount++;
    }
}

byte Schedules::getSchedulesCount() {
    return _schedulesCount;
}

TempSchedule* Schedules::getTempSchedule(byte index) {
    if (index < SCHEDULES_MAX_COUNT) {
        return _schedules[index];
    } else {
        return NULL;
    }
}

