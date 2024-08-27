# Brainstorming

This file is used to document your thoughts, approaches and research conducted across all tasks in the Technical Assessment.

## Firmware
The first thought that comes to mind is to use an existing CAN parsing library. Although this might be the easier way, this does introduce significant
overhead to the parser program. Based on my limited understanding of the software and how RB23's data parsing actually works, if this parser ran directly
in RB23's embedded system (maybe for live low latency CAN data analyzing?) it would be beneficial if it were as small as possible to maximise memory use and efficiency.
Similarly, I will try to minimise the use of external libraries for this exact reason.

First step would be to grab the CAN id and check that it matches ECU_WheelSpeed's DBC id and ignore the line if it doesn't.




## Telemetry

Looks like the app crashes when a random additional '}' appears in the end of the data.

## Cloud