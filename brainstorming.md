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
- WheelSpeedFR's data is in the first 16 bits (the first 2 bytes). The byte order is reversed to get the actual stored value. The scaling factor is then applied.
- This concept is also applied to WheelSpeedRR in the exact same way.

In order to set the output as 1 decimal place as per the spec, a way to do it was to use iomanip's setprecision. This will add extra size, but I noticed that it only adds around 6kb (using ls -l), 
which should be fine. 

And with that, the parser meets spec requirements (parser.cpp). I do want give making a general purpose DBC/CAN parser a try though, I feel like making a parser with tons of hardcoding like this doesn't feel right.

### General DBC/CAN Parser

(genParser.cpp)

Plan:
Split into 2 parts, the DBC parsing and the CAN log file parsing

DBC Parsing:
- Find each CAN frame by finding every line that starts with 'BO_'
- Record each CAN frame ID
- Since the number of signals in a CAN frame is likely to be few, we can just store these in an array.
- All info needed to parse signals (name, start bit, length, endianess, signage, factor, etc) will be stored in this array of strings
- Everything will be stored in a map for fast access

CAN data parsing:
- Basically the same as the original parser made for the spec, but with less hardcoding

DBC Parsing Part:
- Iterate through each line, finding lines starting with 'BO_'
- Parse the CAN Frame id 
- For every line after this one, check if it has the Signal marker 'SG_'
- For every signal, parse the line by separating each string by whitespace and store as string array
- Will end up with string array for every signal -> store in array of string arrays, with each element representing 1 signal
- Create a map entry with the CAN Frame ID as the key, and the array of string arrays as the value
- Repeat for all CAN Frames

CAN Parsing part:
- Parse each line by separating strings by whitespace
- Find the id of each data payload. If no entry in map (no entry in DBC file) then ignore
- Grab info needed for data parsing by extracting each signal's: start bit, bit length, endianess and whether its signed or not
- Use bitmasks to extract relevant data for each signal, manipulating if needed to match endianess and signed-ness
- Do this for every signal

Limitations

This general parser assumes some things:
- The DBC file matches the same format as the one given and its format is correct. This includes each CAN frame in the DBC being separated by at least 1 newline. 
- This means that CAN frames are found by lines starting with 'BO_', and alot of the parsing is done by finding certain markers
that separate data, e.g. whitespace and symbols like | and ,
- The CAN dump/log file is in the format: {timestamp} {interface} {id#payload} all separated by whitespace
- It outputs every data listed in the DBC file, so funny enough this actually doesn't meet spec requirements.

The output of this general parser is in output2.txt

## Telemetry

Looks like the app crashes when a random additional '}' appears in the end of the data. This can be fixed by simply removing the last '}' character
when JSON.parse fails. This is a bad hardcoded fix however, as it only deals with a very specific error case that we have prior knowledge of. 
There is an npm package we can use instead, jsonrepair, that fixes JSON-like strings to become valid JSON. I'll use this instead.

To track whether the temp has reached outside of the accepted range more than 3 times in 5 seconds, we can use a simple counter system with
timers that decrement the counter after 5 seconds. 

I feel like having the error only being logged to console is not as useful as it should be. I think the frontend should also show these error logs. To do this,
I created a section that displays all logs from latest to earliest, and all the logs are stored in an array updated with useState. I'll also make sure 
the web app is at least a little bit responsive.

## Cloud
