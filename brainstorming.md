# Brainstorming

This file is used to document your thoughts, approaches and research conducted across all tasks in the Technical Assessment.

## Firmware
The first thought that comes to mind is to use an existing CAN parsing library. Although this might be the easier way, this does introduce significant
overhead to the parser program. Based on my limited understanding of the software and how RB23's data parsing actually works, if this parser ran directly
in RB23's embedded system (maybe for live low latency CAN data analyzing?) it would be beneficial if it were as small as possible to maximise memory use and efficiency.
Similarly, I will try to minimise the use of external libraries for this exact reason.

First step would be to grab the CAN id and check that it matches ECU_WheelSpeed's DBC id and ignore the line if it doesn't. This can be hardcoded to check if the id matches "705",
which is 1797 in hexadecimal. Looks like most of the parsing can be hardcoded in a way, taking substrings with fixed lenghts and positions

Both WheelSpeedFR and RR's signals (were) written as being in big endian, but it looks like this was wrong. The expected output would match a little endian reading instead (turns out this was the case)

The main data reading can be done by:
- grabbing the data portion (string) then converting it to an unsigned 64 bit integer (uint64_t) using sscanf. After a bit of searching the string must be converted to a C style null terminated
string using .c_str()
- WheelSpeedFR's data is in the first 16 bits (the first 2 bytes). Since the data payload is 8 bytes, we first grab the 2nd byte by shifting it 6 bytes to the right and grabbing the
rightmost byte. This will be our most significant byte (MSB). Then grabbing the 1st byte as our least significant byte (LSB), essentially reversing the order of the bytes to get the number.
The scaling factor is then applied.
- This concept is also applied to WheelSpeedRR in the exact same way.

In order to set the output as 1 decimal place as per the spec, a way to do it was to use iomanip's setprecision. This will add extra size, but I noticed that it only adds around 6kb (using ls -l), 
which should be fine. 

And with that, the parser meets spec requirements. I do want give making a general purpose DBC/CAN parser a try though, I feel like making a parser with tons of hardcoding like this doesn't feel right.

## Telemetry

Looks like the app crashes when a random additional '}' appears in the end of the data. This can be fixed by simply removing the last '}' character
when JSON.parse fails. This is not a good fix however.

To track whether the temp has reached outside of the accepted range more than 3 times in 5 seconds, we can use a simple counter system with
timers that decrement the counter after 5 seconds.

## Cloud