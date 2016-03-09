#ifndef ___FAKEARDUINO_H___
#define ___FAKEARDUINO_H___


/*
 *  This should include the test library.
 *  If not, we generate a compile time error.
 */
#include <Arduino.h>
#ifndef ___ARDUINO_H___
#   error "The Arduino test library is not #included"
#endif


#include <FakeArduinoClock.h>


class FakeArduino {
    public:

        FakeArduino(FakeArduinoClock *clk);
        virtual ~FakeArduino();

        virtual unsigned long millis();
        virtual unsigned long micros();

        //  ---

        FakeArduinoClock *_clock;
};


#endif // ___FAKEARDUINO_H___

