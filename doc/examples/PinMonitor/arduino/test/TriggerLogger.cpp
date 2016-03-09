#include <Arduino.h>
#include <TriggerLogger.h>

#include <stdexcept>


void taskFunction(TaskScheduler::tid_t tid) {
    TriggerLogger::taskMethod(tid);
}

void TriggerLogger::taskMethod(TaskScheduler::tid_t tid) {
    getInstance().add(tid);
}

void TriggerLogger::clearTriggerLog() {
    getInstance()._triggerLog.clear();
    getInstance()._totalNumTriggers = 0;
    getInstance()._lastTriggered = 0;
}

TriggerLogger::TriggerLog& TriggerLogger::getTriggerLog() {
    return getInstance()._triggerLog;
}

TaskScheduler::tid_t TriggerLogger::lastTriggered() {
    if (getInstance()._totalNumTriggers == 0)
        throw std::out_of_range(
                "TriggerLogger::lastTriggered(): No triggers yet");

    return getInstance()._lastTriggered;
}

TriggerLogger& TriggerLogger::getInstance() {
    static TriggerLogger _instance;

    return _instance;
}

void TriggerLogger::add(TaskScheduler::tid_t tid) {
    unsigned long now = millis();

    TriggerLog::iterator taskTimestamps = _triggerLog.find(tid);
    if (taskTimestamps != _triggerLog.end()) {
        taskTimestamps->second.push_back(now);
    } else {
        _triggerLog[tid] = TimestampList();
        _triggerLog[tid].push_back(now);
    }

    _totalNumTriggers++;
    _lastTriggered = tid;
}
