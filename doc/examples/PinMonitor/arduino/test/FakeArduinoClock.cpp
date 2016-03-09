#include <FakeArduinoClock.h>


FakeArduinoClock::FakeArduinoClock() {
    this->reset();
}

FakeArduinoClock::~FakeArduinoClock() {
}

unsigned long FakeArduinoClock::now() {
    return this->_clockus;
}

void FakeArduinoClock::reset() {
    this->_clockus = 0;
}

unsigned long FakeArduinoClock::advanceMillis(unsigned long ms) {
    return this->advanceMicros(1000*ms);
}

unsigned long FakeArduinoClock::advanceMicros(unsigned long us) {
    this->_clockus += us;

    return this->_clockus;
}

