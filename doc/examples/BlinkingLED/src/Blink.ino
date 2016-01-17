#include <BlinkingLED.h>

#define ONBOARD_LED         13
#define LED_ON_DURATION     1000
#define LED_OFF_DURATION    200

BlinkingLED led = BlinkingLED(ONBOARD_LED, LED_ON_DURATION, LED_OFF_DURATION);

void setup() {
    led.setup();
}

void loop() {
    led.blink();
}

