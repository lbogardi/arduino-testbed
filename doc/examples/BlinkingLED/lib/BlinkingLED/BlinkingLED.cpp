#include <Arduino.h>
#include <BlinkingLED.h>

BlinkingLED::BlinkingLED(uint8_t pin,
                         unsigned long on_duration,
                         unsigned long off_duration)
{
    this->pin = pin;
    this->on_duration = on_duration;
    this->off_duration = off_duration;
}

void BlinkingLED::setup()
{
	pinMode(pin, OUTPUT);
}

void BlinkingLED::blink()
{
	digitalWrite(pin, HIGH);
	delay(on_duration);
	digitalWrite(pin, LOW);
	delay(off_duration);
}

