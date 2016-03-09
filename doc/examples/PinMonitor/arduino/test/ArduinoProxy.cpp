#include <ArduinoProxy.h>


ArduinoProxy& ArduinoProxy::getInstance() {
    static ArduinoProxy _instance;

    return _instance;
}

//  The proxy does *not* take ownership of the provided fake!
FakeArduino *ArduinoProxy::fake(FakeArduino *fake) {
    ArduinoProxy& instance = getInstance();

    instance._fake = fake;

    return fake;
}

FakeArduino *ArduinoProxy::fake() {
    return getInstance()._fake;
}

