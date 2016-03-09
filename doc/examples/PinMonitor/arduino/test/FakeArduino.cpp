#include <FakeArduino.h>


//  The object takes ownership of the provided clock!
FakeArduino::FakeArduino(FakeArduinoClock *clk)
{
    this->_clock = clk;
}

FakeArduino::~FakeArduino() {
    delete this->_clock;
}

unsigned long FakeArduino::millis() {
    return this->micros() / 1000;
}

unsigned long FakeArduino::micros() {
    return this->_clock->now();
}

