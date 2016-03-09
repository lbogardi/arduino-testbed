#include <ArduinoProxy.h>


unsigned long millis(void) {
    return ArduinoProxy::fake()->millis();
};

unsigned long micros(void) {
    return ArduinoProxy::fake()->micros();
};

