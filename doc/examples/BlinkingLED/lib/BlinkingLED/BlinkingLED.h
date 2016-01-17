#ifndef ___BLINKINGLED_H___
#define ___BLINKINGLED_H___

class BlinkingLED
{
    private:

        uint8_t pin;
        unsigned long on_duration;
        unsigned long off_duration;

    public:

        BlinkingLED(uint8_t pin,
                    unsigned long on_duration,
                    unsigned long off_duration);

        void setup();
        void blink();
};

#endif // ___BLINKINGLED_H___

