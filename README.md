# MakoMeatHead
* JUCE VST High Gain Amp Sim
* Demo JUCE program. Sample code.
* Tested on Windows only.
* Written in Visual C++ 2022.
* Written for new programmers, not complicated.
* Version: 1.00
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

![Demo Image](docs/assets/meatheaddemo.jpg)

# THEORY OF OPERATION<br />
This VST is designed to mimic a high gain amplifier. It uses
a typical OverDrive/Distortion pedal technique of passing the
guitar signal into a 650 Hz Band Pass filter and then distorting
the signal. A higher frequency push is added as the midrange filter Q is increased.<br />

The basic signal path is:  
Signal Thinner -> High Pass Filter -> High Freq Boost -> 650 Hz Bandpass -> Distortion -> 5 Band EQ -> Thump  

SIGNAL THINNER  
As the gain is increased, the noise floor gets louder and louder. Eventually you start to hear things that are not
supposed to be heard. The thinner reduces the guitar input signal the harder we go into distortion.

HIGH PASS FILTER  
The higher the gain, the less bass you want going into the distortion stage. This filter is set to 150 Hz to match 
the 150 Hz of Thump. But a setting of 180, 200, 220 is not unreasonable. Experiment. The more bass present at distortion, the
more flubby and undefined your sound will be.

HIGH FREQ BOOST  
This code adds in some high frequency signal as the midrange bandpass filter Q is increased. The higher the Q, the less Lows and High 
are present. Without this boost, the sound may be to dead and soft. Experiment.

650 HZ BANDPASS FILTER  
This was selected to give us a honky sounding low mid boost. This works well for 7 string guitars etc. The Q of the filter adjusts the Highs,
Lows, and the WAH sound. The Q setting is crucial to finding the right sound.

DISTORTION STAGE  
The guitar signal is amplified to extreme levels and then clipped. An angled clipping setup is used. It is not necessary, as the
distortion level is ridiculous. So comment it out and try something else.

5 BAND EQ  
As the distortion goes up, the EQ becomes crucial. For typical 7 string setups, you may want to lower EQ bands 2, 3, and 4. Because the
bass and highs are the meat and potatoes of the sound. A Low Pass filter is added and sounds good at around 4000-4800 Hz. If it is set to 8000 Hz
it turns off and lets everything thru.

THUMP  
Thump adds a boosted 150 Hz signal to the sound. A good starting point is to set Thump at around .20 and then adjust the lowest EQ band to taste. Of course,
there is nothing stopping you from going to 11!

# NOTED JUCE CODE 
CLIPPING  
A Timer is created in the EDITOR that gets called twice per second. If the PROCESSOR code detects a clip(volume too loud), it sets a flag. This flag is read
in the Editor Timer and CLIPPING is posted to the UI if true.

This is the basic method you would use for VU meters, etc. Set a value in the processor, detect and display it in the editor TIMER.

SINGLE VALUE MONITORING  
Normally a JUCE slider control is setup to have an associated TEXTBOX object. To give our AMP UI a sleek look we have turned all textboxes off. They are not needed in this VST.
Instead, we have a single LABEL object on the UI that we fill in with our slider data as it is being edited. 

CUSTOM SLIDER UI  
We have an override function declared to take control of the drawing routines for our sliders. Each slider also has a few tick mark settings available. We are cheating and using
some of the sliders color properties to get info about our slider that doesnt exist in the default JUCE control. For instance a slider color is used to indicate the tick mark style to use.

