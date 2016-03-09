#ifndef ___ARDUINO_H___
#define ___ARDUINO_H___


/*
 *  Fake the file guard of the original Arduino.h, so that we can have our
 *  definitions override the real library.
 *
 *  Check that the original library is not yet present, and throw a compile
 *  time error otherwise.
 */
#ifdef Arduino_h
#   error "Fake version of Arduino.h must be #included before the original"
#else
#   define Arduino_h
#endif // Arduino_h


//  Include the standard libraries that Arduino.h includes.
#include <stdlib.h>
#include <string.h>
#include <math.h>

//  Include the Arduino-related headers with typedefs (no functions!)
#include <stdint.h>


//  Fake functions from Arduino.h
unsigned long millis(void);
unsigned long micros(void);


#endif // ___ARDUINO_H___

