# Goldie Clock Firmware #
https://github.com/JChristensen/jcGoldieClock
ReadMe file  
Jack Christensen Sep 2015

## Introduction ##
My modified firmware (Arduino sketch) for LarryD's Goldie Clock. Developed with release 1.0.6 but also tested with 1.6.5.

## Hardware mods ##
INCREMENT button: This is the button on the right-hand side when looking at the clock from the front. It was moved from Arduino pin 2 to pin 4. This makes the INT0 pin (pin 2) available for connection to the RTC SQW pin.

SET button: This is the button on the left side. It was previously called the DECREMENT button. No changes to wiring, still connected to Arduino pin 3.

The DS3231 RTC SQW pin is connected with a short wire to Arduino pin 2.

## Software mods ##
Clock is time-zone and daylight-saving time aware. It will automatically adjust during the transitions between standard and daylight time. This requires the clock setting process to include time zone, year, month and day in addition to the time.

RTC is configured to supply a 1Hz interrupt signal to the MCU. This keeps the firmware accurately in sync with the RTC at all times and alleviates the need to constantly re-sync with the RTC. In reality, this makes almost no practical difference, it's just a little cleaner way of handling the RTC in the firmware.

Button handling during the time setting process is changed to give the user a little more feedback.

Clock face color scheme is somewhat different. The hour hand is now three pixels wide. Strictly personal preference, I find it a little more intuitive and easier to read.

## Setting the clock ##
To enter set mode, hold the SET button (left button, viewing clock from the front) for two seconds. The display will change to magenta hour markers, and the time zone will be indicated by white. Use the right INCREMENT button to set the time zone. Hold the button down to increment rapidly. When setting the time zone, minute positions 0-4 represent UTC, ET, CT, MT, PT.

Once the time zone is set, press and release SET to set the year, indicated by bright magenta. When setting the year, minute positions 15-59 represent years 2015-2059 and minute positions 0-14 represent years 2060-2074.

Press and release SET again to adjust the month. The month is indicated by cyan, and minute positions 1-12 correspond to Jan-Dec.

Press SET again to set the day, indicated by yellow. The minutes 1-31 correspond to the day of the month.

Press SET again to set the hour, indicated by red, in 24-hour format. The minute positions 0-23 correspond to the hours.

Press SET again to set the minute, indicated by blue.

Pressing SET once more will set the seconds to zero and start the clock. If the clock is left in set mode for more than two minutes without buttons being pushed, it will revert to clock mode.

Set mode can be canceled at any time by pressing and holding the SET button. Any partial time setting will be lost, but if the time zone was changed and then SET was pressed again to advance to year setting, the time zone change *will* take effect. This allows the time zone to be changed without affecting the date and time.