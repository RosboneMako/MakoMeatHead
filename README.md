# MakoMeatHead
* JUCE VST High Gain Amp Sim
* Demo JUCE program. Sample code.
* Tested on Windows only.
* Written in Visual C++ 2022.
* Written for new programmers, not complicated.
* Version: 1.10
* Posted: June 27, 2024

VERSION
------------------------------------------------------------------
1.00 - Initial release.

DISCLAIMER
------------------------------------------------------------------  
This VST was written in a very simple way. No object defs, etc. 
Just simple code for people who are not programmers but want to 
dabble in the magic of JUCE VSTs.

SUMMARY
------------------------------------------------------------------
A Juce/C++ VST written to simulate a guitar amplifier. 

![Demo Image](docs/assets/meatheaddemo.png)

# THEORY OF OPERATION<br />
This VST is designed to mimic a high gain amplifier. It uses
a typical OverDrive/Distortion pedal technique of passing the
guitar signal into a 650 Hz Band Pass filter and then distorting
the signal. A higher frequency push is added as the midrange filter Q is increased.<br />

