//main clock class
#ifndef _CLOCK_H
#define _CLOCK_H
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>    //http://github.com/adafruit/Adafruit_NeoPixel
#include <Button.h>               //http://github.com/JChristensen/Button
#include <DS3232RTC.h>            //http://github.com/JChristensen/DS3232RTC
#include <Streaming.h>            //http://arduiniana.org/libraries/streaming/
#include <Time.h>                 //http://playground.arduino.cc/Code/Time
#include <Timezone.h>             //http://github.com/JChristensen/Timezone
#include <avr/eeprom.h>
#include <util/atomic.h>          //http://www.nongnu.org/avr-libc/user-manual/group__util__atomic.html (included with Arduino IDE)

//The Goldie Clock has 60 NeoPixels.
//Looking at the clock, Pixel 0 is the first pixel CCW from the 12:00 position.
//Pixel 14 is at 9:00, Pixel 29 is at 6:00, Pixel 44 is at 3:00, Pixel 59 is at 12:00.

//states for the state machines
enum clockStates_t { RUN_CLOCK, SET_CLOCK };
enum setStates_t { SET_INIT, SET_TZ, SET_YEAR, SET_MON, SET_DAY, SET_HOUR, SET_MIN };

//Clock Face Colors (0xRRGGBB)
const uint32_t SECOND_HAND(0x001800);
const uint32_t MINUTE_HAND(0x000046);
const uint32_t HOUR_HAND(0x500000);
const uint32_t HOUR_HAND_DIM(0x0A0000);
const uint32_t HOUR_MARKER(0x060300);    //hour markers for clock mode
const uint32_t SET_MARKER(0x060003);     //hour markers for set mode

//colors for set mode
const uint32_t RED(0x320000);
const uint32_t YELLOW(0x323200);
const uint32_t GREEN(0x003200);
const uint32_t CYAN(0x003232);
const uint32_t BLUE(0x000032);
const uint32_t MAGENTA(0x320032);
const uint32_t WHITE(0x323232);

//other constants
extern const uint8_t SET_SWITCH_PIN;  //defined in main module
extern const uint8_t INCR_SWITCH_PIN; //defined in main module
const bool PULLUP(true);              //for button objects
const bool INVERT(true);              //for button objects
const uint32_t DEBOUNCE_MS(25);       //debounce time in ms for button objects
const uint32_t REPEAT_FIRST(500);     //ms required before repeating on long press
const uint32_t REPEAT_INCR(150);      //repeat interval for long press
const uint32_t SET_TIMEOUT(120000);   //ms in set mode with no input reverts to clock mode
const uint32_t SET_LONGPRESS(2000);   //ms long press required to enter set mode
const uint16_t FIRST_PIXEL(0), LAST_PIXEL(59);
const uint8_t monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*----------------------------------------------------------------------*
 * GoldieClock class                                                    *
 *----------------------------------------------------------------------*/
class GoldieClock : public Adafruit_NeoPixel
{
public:
    GoldieClock(uint16_t nPixel, uint8_t pin, uint8_t type=NEO_GRB + NEO_KHZ800)
        : Adafruit_NeoPixel(nPixel, pin, type) {}
    void begin(void);
    void run(time_t utc);

private:
    bool setClock(void);
    void displaySet(uint16_t pixel, uint32_t color);
    void displayClock(time_t utc);
    void rainbowCycle(uint8_t wait, uint8_t repeatNumber);
    uint32_t wheel(byte WheelPos);
};

/*---- helper function prototypes ----*/
time_t getUTC(void);
void setUTC(time_t utc);
void printDateTime(time_t t);
void printTime(time_t t);
void printDate(time_t t);
void printI00(int val, char delim);
bool isLeap(int y);

#endif

