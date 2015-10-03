//Gold NeoPixel Clock by Larry Decterow
//Firmware modified by Jack Christensen Sep2015
//
//Hardware changes:
//(1) Move the increment button (looking at the clock, the switch on the right) from pin 2 to pin 4.
//(2) Connect the DS3231 SQW signal to pin 2 so the RTC can interrupt the MCU once per second.
//
//The left switch is known as the SET button, the right switch is still known as INCREMENT.
//
//To set the clock, hold the SET button (left button, viewing clock from the front) for two seconds.
//The display will change to magenta hour markers, and the time zone will be indicated by white.
//Use the right INCREMENT button to set the time zone. Hold the button down to increment rapidly.
//When setting the time zone, minute positions 0-4 represent UTC, ET, CT, MT, PT.
//
//Once the time zone is set, press and release SET to set the year, indicated by bright magenta.
//When setting the year, minute positions 15-59 represent years 2015-2059 and minute positions
//0-14 represent years 2060-2074.
//
//Once the year is set, press and release SET to adjust the month. The month is indicated by cyan,
//and minute positions 1-12 correspond to Jan-Dec.
//
//Press SET again to set the day, indicated by yellow. The minutes 1-31 correspond to the day
//of the month.
//
//Press SET again to set the hour, indicated by red, in 24-hour format. The minute positions 0-23
//correspond to the hours.
//
//Press SET again to set the minute, indicated by blue.
//
//Pressing SET once more will set the seconds to zero and start the clock. If the clock is left
//in set mode for more than two minutes it will revert to clock mode.
//
//Set mode can be canceled at any time by pressing and holding the SET button. Any partial time
//setting will be lost, but if the time zone was changed and then SET is pressed again to advance
//to year setting, the time zone change *will* take effect. This allows the time zone to be changed
//without affecting the date and time.
//
//Rainbow "chimes" occur on the quarter hour. Rainbows are on by default after power up.
//Rainbows can be turned off by pressing and releasing the INCREMENT button when the clock
//is not in set mode. The display will go blank for a second to indicate rainbows are off,
//or will display a rainbow to show rainbows are on.

#include <Adafruit_NeoPixel.h>    //http://github.com/adafruit/Adafruit_NeoPixel
#include <Button.h>               //http://github.com/JChristensen/Button
#include <DS3232RTC.h>            //http://github.com/JChristensen/DS3232RTC
#include <Streaming.h>            //http://arduiniana.org/libraries/streaming/
#include <Time.h>                 //http://playground.arduino.cc/Code/Time
#include <Timezone.h>             //http://github.com/JChristensen/Timezone
#include <Wire.h>                 //http://arduino.cc/en/Reference/Wire
#include "classes.h"
#include "clock.h"

//pin definitions
const uint8_t RXD_PIN(0);
const uint8_t TXD_PIN(1);
const uint8_t RTC_1HZ_PIN(2);
const uint8_t SET_SWITCH_PIN(3);      //on the left, looking at the clock from the front
const uint8_t INCR_SWITCH_PIN(4);     //on the right, looking at the clock from the front
const uint8_t NEO_PIXEL_PIN(6);
const uint8_t HB_LED_PIN(13);
const uint8_t SDA_PIN(A4);
const uint8_t SCL_PIN(A5);

const uint8_t UNUSED_PINS[] = { 5, 7, 8, 9, 10, 11, 12, A0, A1, A2, A3 };

//other constants
const uint16_t N_PIXEL(60);           //number of pixels

//object instantiations
GoldieClock clock(N_PIXEL, NEO_PIXEL_PIN);
heartbeat hbLED(HB_LED_PIN, 1000);

void setup(void)
{
    //enable pullups on unused pins for noise immunity
    for ( uint8_t i=0; i<sizeof(UNUSED_PINS); ++i )
        pinMode(UNUSED_PINS[i], INPUT_PULLUP);

    Serial.begin(115200);
    Serial << F( "\n" __FILE__ " " __DATE__ " " __TIME__ "\n" );

    pinMode(RTC_1HZ_PIN, INPUT_PULLUP);         //enable pullup on interrupt pin (RTC SQW pin is open drain)
    EICRA = _BV(ISC01);                         //INT0 on falling edge
    EIFR |= _BV(INTF0);                         //ensure interrupt flag is cleared
    EIMSK |= _BV(INT0);                         //enable INT0
    RTC.squareWave(SQWAVE_1_HZ);                //1 Hz square wave

    clock.begin();
    hbLED.begin();
}

void loop(void)
{
    clock.run( getUTC() );
    hbLED.update();                             //(blink 'em if ya got 'em)
}

