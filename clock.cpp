//main clock class
#include "clock.h"

//object instantiations
Button btnSet(INCR_SWITCH_PIN, DEBOUNCE_MS);
Button btnIncr(SET_SWITCH_PIN, DEBOUNCE_MS);

//time zones
TimeChangeRule EDT = { "EDT", Second, Sun, Mar, 2, -240 };    //Daylight time = UTC - 4 hours
TimeChangeRule EST = { "EST", First, Sun, Nov, 2, -300 };     //Standard time = UTC - 5 hours
Timezone Eastern(EDT, EST);
TimeChangeRule CDT = { "CDT", Second, Sun, Mar, 2, -300 };    //Daylight time = UTC - 5 hours
TimeChangeRule CST = { "CST", First, Sun, Nov, 2, -360 };     //Standard time = UTC - 6 hours
Timezone Central(CDT, CST);
TimeChangeRule MDT = { "MDT", Second, Sun, Mar, 2, -360 };    //Daylight time = UTC - 6 hours
TimeChangeRule MST = { "MST", First, Sun, Nov, 2, -420 };     //Standard time = UTC - 7 hours
Timezone Mountain(MDT, MST);
TimeChangeRule PDT = { "PDT", Second, Sun, Mar, 2, -420 };    //Daylight time = UTC - 7 hours
TimeChangeRule PST = { "PST", First, Sun, Nov, 2, -480 };     //Standard time = UTC - 8 hours
Timezone Pacific(PDT, PST);
TimeChangeRule utcRule = { "UTC", First, Sun, Nov, 2, 0 };    //No change for UTC
Timezone UTC(utcRule, utcRule);
Timezone* timezones[] = { &UTC, &Eastern, &Central, &Mountain, &Pacific };
const char* tzNames[] = { "UTC", "Eastern", "Central", "Mountain", "Pacific" };
Timezone* tz;                       //pointer to the time zone
uint8_t tzIndex;                    //index to the timezones[] array and the tzNames[] array
EEMEM uint8_t ee_tzIndex;           //copy persisted in EEPROM
TimeChangeRule* tcr;                //pointer to the time change rule, use to get TZ abbrev

//initialize
void GoldieClock::begin(void)
{
    btnSet.begin();
    btnIncr.begin();
    Adafruit_NeoPixel::begin();

    //get the time zone index from eeprom and ensure that it's valid
    tzIndex = eeprom_read_byte( &ee_tzIndex );
    if ( tzIndex >= sizeof(tzNames) / sizeof(tzNames[0]) )
    {
        tzIndex = 0;                            //not valid, set to UTC
        eeprom_write_byte( &ee_tzIndex, tzIndex);
    }
    tz = timezones[tzIndex];                    //set the tz

    time_t utc = getUTC();                      //synchronize with RTC
    while ( utc == getUTC() );                  //wait for increment to the next second
    utc = RTC.get();
    setUTC(utc);                                //set our time from the RTC
    Serial << F("\nTime set from RTC:\n");
    printDateTime(utc);
    Serial << F("UTC") << endl;
    time_t local = (*tz).toLocal(utc, &tcr);
    printDateTime(local);
    Serial << tcr -> abbrev << endl;

//    rainbowCycle(2, 2);                         //power-up eye candy
    clear();                                    //turn all the NeoPixels off at power up
    show();
}

//run the clock state machine. call this function frequently from the main loop.
void GoldieClock::run(time_t utc)
{
    static clockStates_t CLOCK_STATE;

    btnSet.read();
    btnIncr.read();

    switch ( CLOCK_STATE)
    {
    case RUN_CLOCK:
        if ( btnSet.pressedFor(SET_LONGPRESS) )
        {
            CLOCK_STATE = SET_CLOCK;
        }
        else if ( btnIncr.wasReleased() )      //toggle the quarter-hour rainbows
        {
            if ( _showRainbows = !_showRainbows )
            {
                rainbowCycle(2, 1);            //show a rainbow if rainbow mode is on
            }
            else
            {
                clear();                       //else just blank the display for a second
                show();
                delay(1000);
            }
        }
        else
        {
            displayClock(utc);
        }
        break;

    case SET_CLOCK:
        if ( setClock() )    //returns true when setting complete
        {
            CLOCK_STATE = RUN_CLOCK;
        }
        break;
    }
}

//run the time setting state machine. must be called frequently while in set mode.
//returns true when setting is complete or has timed out.
bool GoldieClock::setClock(void)
{
    static setStates_t SET_STATE;
    static time_t utc, local;
    static uint8_t newTzIndex;
    static int y, mth, d, h, m, maxDays;
    static uint16_t pixel;
    static uint32_t rpt;
    bool retval = false;

    //see if user wants to cancel set mode or if set mode has timed out
    if ( SET_STATE != SET_INIT )
    {
        if ( btnSet.pressedFor(SET_LONGPRESS) )
        {
            SET_STATE = SET_INIT;
            displayClock( getUTC() );
            while ( btnSet.isPressed() ) btnSet.read();    //wait for user to release the button
            return true;
        }
        uint32_t ms = millis();
        if ( (ms - btnSet.lastChange() >= SET_TIMEOUT) && (ms - btnIncr.lastChange() >= SET_TIMEOUT) )
        {
            SET_STATE = SET_INIT;
            return true;
        }
    }

    switch ( SET_STATE )
    {
    case SET_INIT:
        SET_STATE = SET_TZ;
        rpt = REPEAT_FIRST;
        utc = getUTC();
        local = (*tz).toLocal(utc, &tcr);
        newTzIndex = tzIndex;
        displaySet( newTzIndex, WHITE );
        while ( btnSet.isPressed() ) btnSet.read();    //wait for user to release the button
        break;

    case SET_TZ:
        if ( btnSet.wasReleased() )                    //then move on to set year
        {
            SET_STATE = SET_YEAR;
            rpt = REPEAT_FIRST;
            if ( newTzIndex != tzIndex )
            {
                tzIndex = newTzIndex;
                tz = timezones[tzIndex];
                eeprom_write_byte( &ee_tzIndex, tzIndex);
                Serial << F("Time zone changed to ") << tzNames[tzIndex] << endl;
            }
            y = year(local);
            if ( y < 2015 || y > 2074 ) y = 2015;      //year must be in range 2015-2074
            pixel = y - 2000;                          //map to pixel
            if ( pixel > 60 ) pixel -= 60;
            displaySet( pixel, MAGENTA );
        }
        else if ( btnIncr.wasReleased() )
        {
            rpt = REPEAT_FIRST;
            if ( ++newTzIndex >= sizeof(tzNames) / sizeof(tzNames[0]) ) newTzIndex = 0;
            displaySet( newTzIndex, WHITE );
        }
        else if ( btnIncr.pressedFor(rpt) )
        {
            rpt += REPEAT_INCR;
            if ( ++newTzIndex >= sizeof(tzNames) / sizeof(tzNames[0]) ) newTzIndex = 0;
            displaySet( newTzIndex, WHITE );
        }
        break;

    case SET_YEAR:
        if ( btnSet.wasReleased() )
        {
            SET_STATE = SET_MON;
            rpt = REPEAT_FIRST;
            mth = month(local);
            displaySet( mth, CYAN );
        }
        else if ( btnIncr.wasReleased() )
        {
            rpt = REPEAT_FIRST;
            if ( ++pixel > LAST_PIXEL ) pixel = 0;
            displaySet( pixel, MAGENTA );
            y = pixel < 15 ? 2060 + pixel : 2000 + pixel;    //map pixel to year
        }
        else if ( btnIncr.pressedFor(rpt) )
        {
            rpt += REPEAT_INCR;
            if ( ++pixel > LAST_PIXEL ) pixel = 0;
            displaySet( pixel, MAGENTA );
            y = pixel < 15 ? 2060 + pixel : 2000 + pixel;    //map pixel to year
        }
        break;

    case SET_MON:
        if ( btnSet.wasReleased() )
        {
            SET_STATE = SET_DAY;
            rpt = REPEAT_FIRST;
            d = day(local);
            maxDays = monthDays[mth-1];                      //number of days in the month
            if (mth == 2 && isLeap(y)) ++maxDays;            //account for leap year
            if ( d > maxDays ) d = maxDays;
            displaySet( d, YELLOW );
        }
        else if ( btnIncr.wasReleased() )
        {
            rpt = REPEAT_FIRST;
            if ( ++mth > 12 ) mth = 1;                       //wrap from dec back to jan
            displaySet( mth, CYAN );
        }
        else if ( btnIncr.pressedFor(rpt) )
        {
            rpt += REPEAT_INCR;
            if ( ++mth > 12 ) mth = 1;
            displaySet( mth, CYAN );
        }
        break;

    case SET_DAY:
        if ( btnSet.wasReleased() )
        {
            SET_STATE = SET_HOUR;
            rpt = REPEAT_FIRST;
            h = hour(local);
            displaySet( h, RED );
        }
        else if ( btnIncr.wasReleased() )
        {
            rpt = REPEAT_FIRST;
            if ( ++d > maxDays ) d = 1;
            displaySet( d, YELLOW );
        }
        else if ( btnIncr.pressedFor(rpt) )
        {
            rpt += REPEAT_INCR;
            if ( ++d > maxDays ) d = 1;
            displaySet( d, YELLOW );
        }
        break;

    case SET_HOUR:
        if ( btnSet.wasReleased() )
        {
            SET_STATE = SET_MIN;
            rpt = REPEAT_FIRST;
            m = minute(local);
            displaySet( m, BLUE );
        }
        else if ( btnIncr.wasReleased() )
        {
            rpt = REPEAT_FIRST;
            if ( ++h > 23 ) h = 0;
            displaySet( h, RED );
        }
        else if ( btnIncr.pressedFor(rpt) )
        {
            rpt += REPEAT_INCR;
            if ( ++h > 23 ) h = 0;
            displaySet( h, RED );
        }
        break;

    case SET_MIN:
        if ( btnSet.wasReleased() )
        {
            SET_STATE = SET_INIT;
            rpt = REPEAT_FIRST;
            tmElements_t tm;
            tm.Year = CalendarYrToTm(y);
            tm.Month = mth;
            tm.Day = d;
            tm.Hour = h;
            tm.Minute = m;
            tm.Second = 0;
            local = makeTime(tm);
            utc = (*tz).toUTC(local);
            RTC.set(utc);
            setUTC(utc);
            Serial << F("\nTime set to:\n");
            printDateTime(utc);
            Serial << F("UTC") << endl;
            printDateTime(local);
            Serial << tcr -> abbrev << endl;
            retval = true;
        }
        else if ( btnIncr.wasReleased() )
        {
            rpt = REPEAT_FIRST;
            if ( ++m > 59 ) m = 0;
            displaySet( m, BLUE );
        }
        else if ( btnIncr.pressedFor(rpt) )
        {
            rpt += REPEAT_INCR;
            if ( ++m > 59 ) m = 0;
            displaySet( m, BLUE );
        }
        break;
    }
    return retval;
}

//update the display while setting the date/time
void GoldieClock::displaySet(uint16_t pixel, uint32_t color)
{
    clear();
    for (uint16_t i = 0; i <= LAST_PIXEL; i += 5)    //hour markers
        setPixelColor(LAST_PIXEL - i, SET_MARKER);
    setPixelColor(LAST_PIXEL - pixel, color);
    show();
}

//display the given time (if different from time given with prior call)
void GoldieClock::displayClock(time_t utc)
{
    static time_t utcLast;
    if ( utc != utcLast )
    {
        utcLast = utc;
        time_t local = (*tz).toLocal(utc, &tcr);
        uint8_t h = hour(local);
        uint8_t m = minute(local);
        uint8_t s = second(local);
        uint8_t hourHand = ( h >= 12 ? h - 12 : h ) * 5;
        hourHand = hourHand + m / 12;      //adjust hour hand between hours

        //one rainbow on the quarter hour, two on the half, four on the hour
        if ( s == 0  && _showRainbows )
        {
            if ( m == 15 || m == 45 ) rainbowCycle(2, 1);
            else if ( m == 30 ) rainbowCycle(2, 2);
            else if ( m == 0 ) rainbowCycle(2, 4);
        }

        clear();

        //hour hand (3 pixels wide)
        setPixelColor( LAST_PIXEL - hourHand, HOUR_HAND );
        setPixelColor( LAST_PIXEL - (hourHand + 1U > LAST_PIXEL ? FIRST_PIXEL : hourHand + 1U), HOUR_HAND_DIM );
        setPixelColor( LAST_PIXEL - (hourHand - 1U > LAST_PIXEL ? LAST_PIXEL : hourHand - 1U), HOUR_HAND_DIM );

        //minute hand & second hand -- additive colors
        setPixelColor( LAST_PIXEL - m, getPixelColor(LAST_PIXEL - m) + MINUTE_HAND );
        setPixelColor( LAST_PIXEL - s, getPixelColor(LAST_PIXEL - s) + SECOND_HAND );

        //hour markers (do not overlay hands)
        for (uint16_t i = 0; i <= LAST_PIXEL; i += 5)
            if ( getPixelColor(LAST_PIXEL - i) == 0 ) setPixelColor(LAST_PIXEL - i, HOUR_MARKER);

        show();
    }
}

//                            r a i n b o w C y c l e ( )
//=================================================================================
//From Adafruit
// Slightly different, this makes the rainbow equally distributed throughout
//
void GoldieClock::rainbowCycle(uint8_t wait, uint8_t repeatNumber)
{
    for(uint16_t j = 0; j < 256U * repeatNumber; j++)
    { // repeatNumber of cycles of all colors on wheel
        for(uint16_t i=0; i< numPixels(); i++)
        {
            setPixelColor(i, wheel(((i * 256 / numPixels()) + j) & 255));
        }
        show();
        delay(wait);
    }
}

//                                 W h e e l ( )
//=================================================================================
// From Adafruit
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
//
uint32_t GoldieClock::wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if (WheelPos < 170)
    {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

/*---- helper functions ----*/

volatile time_t isrUTC;                         //ISR's copy of current time in UTC

//return current time
time_t getUTC(void)
{
    time_t utc;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        utc = isrUTC;
    }
    return utc;
}

//set the current time
void setUTC(time_t utc)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        isrUTC = utc;
    }
}

//1Hz RTC interrupt handler increments the current time
ISR(INT0_vect)
{
    ++isrUTC;    //increment the time
}

//print date and time to Serial
void printDateTime(time_t t)
{
    printDate(t);
    Serial << ' ';
    printTime(t);
}

//print time to Serial
void printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

//print date to Serial
void printDate(time_t t)
{
    printI00(day(t), 0);
    Serial << monthShortStr(month(t)) << _DEC(year(t));
}

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void printI00(int val, char delim)
{
    if (val < 10) Serial << '0';
    Serial << _DEC(val);
    if (delim > 0) Serial << delim;
    return;
}

//Leap years are those divisible by 4, but not those divisible by 100,
//except that those divisble by 400 *are* leap years.
//See Kernighan & Ritchie, 2nd edition, section 2.5.
bool isLeap(int y)
{
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

