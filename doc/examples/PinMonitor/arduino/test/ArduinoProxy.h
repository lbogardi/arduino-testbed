#ifndef ___ARDUINOPROXY_H___
#define ___ARDUINOPROXY_H___


#include <FakeArduino.h>


class ArduinoProxy {
    public:

        static FakeArduino *fake(FakeArduino *fake);
        static FakeArduino *fake();

    private:

        FakeArduino *_fake;

        //  -----

        ArduinoProxy() {};
        ArduinoProxy(ArduinoProxy const&);
        void operator=(ArduinoProxy const&);

        static ArduinoProxy& getInstance();
};


#endif // ___ARDUINOPROXY_H___

