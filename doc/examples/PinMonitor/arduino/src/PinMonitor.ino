#include <TaskScheduler.h>


#define NUMBER_OF_TASKS             4

#define SERIAL_BITRATE              9600
#define SERIAL_OPEN_DELAY_MILLIS    1000
#define TRANSMISSION_FREQ           500

#define BLINK_FREQ                  1000
#define BLINK_LED_PIN               13

#define ANALOG_READ_FREQ            100
#define LEFT_ANALOG_PIN             A1
#define RIGHT_ANALOG_PIN            A0

#define DIGITAL_READ_FREQ           100
#define LEFT_DIGITAL_PIN            2
#define RIGHT_DIGITAL_PIN           4

//  -----

typedef struct {
    unsigned long   nowMillis;
    unsigned long   nowMicros;
    unsigned short  leftAnalog;
    unsigned short  rightAnalog;
    bool            leftDigital;
    bool            rightDigital;
    bool            ledState;
} status_t;

status_t status;

void transmit_status(TaskScheduler::tid_t tid) {
    status.nowMillis = millis();
    status.nowMicros = micros();

    Serial.write((const uint8_t *)&status, sizeof(status_t));
}

void switch_led_state(TaskScheduler::tid_t tid) {
    status.ledState = !status.ledState;
    digitalWrite(BLINK_LED_PIN, status.ledState);
}

class PinReader {
    public:

        static void readAnalog(TaskScheduler::tid_t tid);
        static void readDigital(TaskScheduler::tid_t tid);
};

void PinReader::readAnalog(TaskScheduler::tid_t tid) {
    status.leftAnalog   = analogRead(LEFT_ANALOG_PIN);
    status.rightAnalog  = analogRead(RIGHT_ANALOG_PIN);
}

void PinReader::readDigital(TaskScheduler::tid_t tid) {
    status.leftDigital  = digitalRead(LEFT_DIGITAL_PIN);
    status.rightDigital = digitalRead(RIGHT_DIGITAL_PIN);
}

//  -----

TaskScheduler::task_t schedule[NUMBER_OF_TASKS] = {
    {true,  BLINK_FREQ,         0,  switch_led_state},
    {true,  ANALOG_READ_FREQ,   0,  PinReader::readAnalog},
    {true,  DIGITAL_READ_FREQ,  0,  PinReader::readDigital},
    {true,  TRANSMISSION_FREQ,  0,  transmit_status},
};

TaskScheduler scheduler(NUMBER_OF_TASKS, schedule);

//  -----

void setup() {
    status = {0};

    while (millis() < SERIAL_OPEN_DELAY_MILLIS);
    Serial.begin(SERIAL_BITRATE);
}

void loop() {
    scheduler.tick();
}

