#ifndef ___FAKEARDUINOCLOCK_H___
#define ___FAKEARDUINOCLOCK_H___


class FakeArduinoClock {
    public:

        FakeArduinoClock();
        virtual ~FakeArduinoClock();

        virtual unsigned long now();
        virtual void reset();

        virtual unsigned long advanceMillis(unsigned long ms);
        virtual unsigned long advanceMicros(unsigned long us);

    protected:

        unsigned long _clockus;
};


#endif // ___FAKEARDUINOCLOCK_H___

