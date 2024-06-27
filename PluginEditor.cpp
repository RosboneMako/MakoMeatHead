/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MakoBiteAudioProcessorEditor::MakoBiteAudioProcessorEditor (MakoBiteAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{    
    char Mess[40];

    //R1.00 Create SLIDER ATTACHMENTS so our parameter vars get adjusted automatically for Get/Set states.
    ParAtt[e_Gain] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "gain", sldKnob[e_Gain]);
    ParAtt[e_NGate] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "ngate", sldKnob[e_NGate]);
    ParAtt[e_Drive] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "drive", sldKnob[e_Drive]);
    ParAtt[e_MidQ] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "midq", sldKnob[e_MidQ]);
    ParAtt[e_EQ] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq", sldKnob[e_EQ]);
    ParAtt[e_EQ1] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq1", sldKnob[e_EQ1]);
    ParAtt[e_EQ2] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq2", sldKnob[e_EQ2]);
    ParAtt[e_EQ3] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq3", sldKnob[e_EQ3]);
    ParAtt[e_EQ4] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq4", sldKnob[e_EQ4]);
    ParAtt[e_EQ5] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "eq5", sldKnob[e_EQ5]);
    ParAtt[e_IR] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "ir", sldKnob[e_IR]);
    ParAtt[e_Mono] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "mono", sldKnob[e_Mono]);
    ParAtt[e_Thump] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "thump", sldKnob[e_Thump]);
    ParAtt[e_HighCut] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "highcut", sldKnob[e_HighCut]);
        
    imgBackground = juce::ImageCache::getFromMemory(BinaryData::meatheadback_png, BinaryData::meatheadback_pngSize);

    //R1.00 Start our Timer so we can tell the user they are clipping. Could draw VU Meters here, etc.
    startTimerHz(2);  //R1.00 have our Timer get called twice per second.

    //****************************************************************************************
    //R1.00 Add GUI CONTROLS
    //****************************************************************************************
    GUI_Init_Large_Slider(&sldKnob[e_Gain], audioProcessor.Setting[e_Gain],0.0f, 1.0f,.01f,"", 1, 0xFFFF0000);
    GUI_Init_Large_Slider(&sldKnob[e_NGate], audioProcessor.Setting[e_NGate], 0.0f, 1.0f, .01f, "", 1, 0xFFFF0000);
    GUI_Init_Large_Slider(&sldKnob[e_IR], audioProcessor.Setting[e_IR], 0, 5, 1, "", 3, 0xFFFF0000);
    GUI_Init_Large_Slider(&sldKnob[e_Drive], audioProcessor.Setting[e_Drive], 0.0f, 1.0f, .01f, "", 1, 0xFFFF0000);
    GUI_Init_Large_Slider(&sldKnob[e_MidQ], audioProcessor.Setting[e_MidQ], 0.1f, 3.0f, .1f, "", 1, 0xFFFF0000);
    GUI_Init_Large_Slider(&sldKnob[e_EQ], audioProcessor.Setting[e_EQ], 0, 10, 1, "", 1, 0xFFFF8000);
    GUI_Init_Large_Slider(&sldKnob[e_EQ1], audioProcessor.Setting[e_EQ1], -12.0f, 12.0f, .1f, "", 2, 0xFFFF8000);
    GUI_Init_Large_Slider(&sldKnob[e_EQ2], audioProcessor.Setting[e_EQ2], -12.0f, 12.0f, .1f, "", 2, 0xFFFF8000);
    GUI_Init_Large_Slider(&sldKnob[e_EQ3], audioProcessor.Setting[e_EQ3], -12.0f, 12.0f, .1f, "", 2, 0xFFFF8000);
    GUI_Init_Large_Slider(&sldKnob[e_EQ4], audioProcessor.Setting[e_EQ4], -12.0f, 12.0f, .1f, "", 2, 0xFFFF8000);
    GUI_Init_Large_Slider(&sldKnob[e_EQ5], audioProcessor.Setting[e_EQ5], -12.0f, 12.0f, .1f, "", 2, 0xFFFF8000);
    
    GUI_Init_Small_Slider(&sldKnob[e_Mono], audioProcessor.Setting[e_Mono], 0, 1, 1, "");    
    GUI_Init_Small_Slider(&sldKnob[e_Thump], audioProcessor.Setting[e_Thump], 0.0f, 1.0f, .01f, "");
    GUI_Init_Small_Slider(&sldKnob[e_HighCut], audioProcessor.Setting[e_HighCut], 2000, 8000, 100, "");
        
    //R1.00 Update the Look and Feel (Global colors) so drop down menu is the correct color. 
    getLookAndFeel().setColour(juce::DocumentWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::DocumentWindow::textColourId, juce::Colour(255, 255, 255));
    getLookAndFeel().setColour(juce::DialogWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(192, 0, 0));
    getLookAndFeel().setColour(juce::TextButton::buttonOnColourId, juce::Colour(192, 0, 0));
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ListBox::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::Label::backgroundColourId, juce::Colour(32, 32, 32));

    //R1.00 Define our control positions to make drawing easier.
    KNOB_DefinePosition(e_Gain,  45,  10, 50, 50, 0, "Gain","");
    KNOB_DefinePosition(e_NGate, 45,  65, 45, 45, 0, "Gate","");
    KNOB_DefinePosition(e_IR,    45, 115, 45, 45, 1, "IR Model","");

    KNOB_DefinePosition(e_EQ1,    90, 65, 60, 60, 0, "150", " dB");
    KNOB_DefinePosition(e_EQ2,   150, 65, 60, 60, 0, "300", " dB");
    KNOB_DefinePosition(e_EQ3,   210, 65, 60, 60, 0, "750", " dB");
    KNOB_DefinePosition(e_EQ4,   270, 65, 60, 60, 0, "1500", " dB");
    KNOB_DefinePosition(e_EQ5,   330, 65, 60, 60, 0, "3000", " dB");
    
    KNOB_DefinePosition(e_Drive, 385,  10, 50, 50, 0, "Drive", "");
    KNOB_DefinePosition(e_EQ,    390,  65, 45, 45, 1, "EQ Band", "");
    KNOB_DefinePosition(e_MidQ,  390, 115, 45, 45, 2, "Mid Q", "");

    KNOB_DefinePosition(e_Mono,    150, 133, 60, 20, 4, "Stereo/Mono", "");
    KNOB_DefinePosition(e_Thump,   210, 133, 60, 20, 0, "Thump", "");
    KNOB_DefinePosition(e_HighCut, 270, 133, 60, 20, 3, "High Cut", " Hz");

    Knob_Cnt = 15;

    //R1.00 THUMB label.
    labInfo1.setJustificationType(juce::Justification::centredLeft);
    labInfo1.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF000000));
    labInfo1.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8000));
    labInfo1.setColour(juce::Label::outlineColourId, juce::Colour(0xFF402000));
    addAndMakeVisible(labInfo1);
    
    //R1.00 HIGH CUT label.
    labInfo2.setJustificationType(juce::Justification::centredRight);
    labInfo2.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF000000));
    labInfo2.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8000));
    labInfo2.setColour(juce::Label::outlineColourId, juce::Colour(0xFF402000));
    addAndMakeVisible(labInfo2);
    
    //R1.00 Update our filter text strings.
    Band_SetFilterValues(false);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    //R1.00 Set the window size.
    setSize(GUI_Width, GUI_Height);
}

MakoBiteAudioProcessorEditor::~MakoBiteAudioProcessorEditor()
{
}

//R1.00 This func gets called twice a second to monitor processor.
//R1.00 Could be used to draw VU meters, etc. 
void MakoBiteAudioProcessorEditor::timerCallback()
{
    //R1.00 Check processor if audio is clipping.
    //R1.00 Track the Label stats so we are not redrawing the control twice a second.
    if (audioProcessor.AudioIsClipping)
    {
        audioProcessor.AudioIsClipping = false;
        STATE_Clip = true;
        labInfo2.setText("CLIPPING", juce::sendNotification);
    }
    else
    {
        if (STATE_Clip) labInfo2.setText("", juce::sendNotification);
        STATE_Clip = false;
    }
}

//==============================================================================
void MakoBiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    bool UseImage = true;   

    if (UseImage)
    {
        g.drawImageAt(imgBackground, 0, 0);        
        
        //R1.00 Use to get control positions when making an image background.
        //g.setColour(juce::Colours::white);
        //g.fillRect(0, 0, GUI_Width, GUI_Height);
    }
    else
    {        
        //R1.00 Draw our GUI.
        //R1.00 Background.
        juce::ColourGradient ColGrad;
        ColGrad = juce::ColourGradient(juce::Colour(0xFF202020), 0.0f, 0.0f, juce::Colour(0xFF808080), 0.0f, GUI_Height, false);
        g.setGradientFill(ColGrad);
        g.fillRect(0, 0, GUI_Width, GUI_Height);

        //R1.00 Draw LOGO text.
        g.setColour(juce::Colours::black);
        g.fillRect(120, 0, 240, 50);
        g.setFont(18.0f);
        g.setColour(juce::Colours::white);
        g.drawFittedText("MEAT*HEAD", 120, 0, 240, 18, juce::Justification::centred, 1);
        g.setFont(14.0f);
        g.setColour(juce::Colours::orange);
        g.drawFittedText("M * A * K * O", 120, 15, 240, 15, juce::Justification::centred, 1);       

        g.setColour(juce::Colour(0x80000000));
        g.fillRect(10, 10, 20, 140);
        g.fillRect(450, 10, 20, 140);
                
        g.setColour(juce::Colours::red);
        g.fillEllipse(112, 137, 11, 11);
        g.setColour(juce::Colours::black);
        g.drawEllipse(110, 135, 15, 15, 3);

    }
    
    //R1.00 DRAW TEXT.
    //R1.00 Most of these could be done on the image to speed up painting.
    //R1.00 But the EQ frequencies need to update. So we are doing all the text
    //R1.00 here so the font matches no matter what in the future.
    g.setFont(12.0f);
    g.setColour(juce::Colours::orange);
    for (int t = 0; t < Knob_Cnt; t++)
    {
       g.drawFittedText(Knob_Info[t].Name, Knob_Info[t].x, Knob_Info[t].y - 10, Knob_Info[t].sizex, 15, juce::Justification::centred, 1);
    }

    //R1.00 Draw IR OFF setting.
    g.drawFittedText("off", 36, 144, 30, 15, juce::Justification::centredLeft, 1);

}

void MakoBiteAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //R1.00 Use RETURN here to have a blank window drawn when creating a GUI image.
    //R1.00 The controls are only visible if thier BOUNDS are defined.
    //return;

    //R1.00 Draw all of the defined KNOBS.
    for (int t = 0; t < Knob_Cnt; t++) sldKnob[t].setBounds(Knob_Info[t].x, Knob_Info[t].y, Knob_Info[t].sizex, Knob_Info[t].sizey);

    labInfo1.setBounds(125, 30, 80, 15);
    labInfo2.setBounds(275, 30, 80, 15);
}


//R1.00 Setup the SLIDER control edit values, Text Suffix (if any), UI tick marks, and Indicator Color.
void MakoBiteAudioProcessorEditor::GUI_Init_Large_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix, int TickStyle, int ThumbColor)
{
    //R1.00 Setup the slider edit parameters.
    slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    slider->setTextValueSuffix(Suffix);
    slider->setRange(Vmin, Vmax, Vinterval);
    slider->setValue(Val);
    slider->addListener(this);
    addAndMakeVisible(slider);

    //R1.00 Override the default Juce drawing routines and use ours.
    slider->setLookAndFeel(&myLookAndFeel);

    //R1.00 Setup the type and colors for the sliders.
    slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFC08000));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFF804000));
    slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0x00000000));    //R1.00 Make this SEE THRU. Alpha=0.
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(ThumbColor));

    //R1.00 Cheat: We are using this color as a Tick Mark style selector in our drawing function.
    slider->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(TickStyle));
}

void MakoBiteAudioProcessorEditor::GUI_Init_Small_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix)
{
    //R1.00 Setup the slider edit parameters.
    slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    slider->setRange(Vmin, Vmax, Vinterval);
    slider->setTextValueSuffix(Suffix);
    slider->setValue(Val);
    slider->addListener(this);
    addAndMakeVisible(slider);

    //R1.00 Setup the type and colors for the sliders.
    slider->setSliderStyle(juce::Slider::LinearHorizontal);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFA0A0A0));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF202020));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::trackColourId, juce::Colour(0xFF603000));
    slider->setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(0xFFE02020));
}

void MakoBiteAudioProcessorEditor::KNOB_DefinePosition(int idx,float x, float y, float sizex, float sizey, int datatype, juce::String name, juce::String suffix)
{
    Knob_Info[idx].x = x;
    Knob_Info[idx].y = y;
    Knob_Info[idx].sizex = sizex;
    Knob_Info[idx].sizey = sizey;
    Knob_Info[idx].DataType = datatype;
    Knob_Info[idx].Suffix = suffix;
    Knob_Info[idx].Name = name;
}

//R1.00 Func to format our SLIDER settings to print on the UI (labInfo1).
void MakoBiteAudioProcessorEditor::KNOB_ShowValue(int t)
{
    char Mess[40];
    juce::String Mess2;
    
    switch (Knob_Info[t].DataType)
    {
      case 0: //R1.00 Generic FLOAT value.
        sprintf(Mess, "%1.2f", audioProcessor.Setting[t]);
        labInfo1.setText(Mess + Knob_Info[t].Suffix, juce::dontSendNotification);
        break;

      case 1: //R1.00 Generic INT value.
         Mess2 = std::to_string(int(audioProcessor.Setting[t]));
         labInfo1.setText(Mess2 + Knob_Info[t].Suffix, juce::sendNotification);
         break;

      case 2:  //R1.00 Print Q val with 2 decimal places.
        sprintf(Mess, "%1.2f", audioProcessor.Setting[t]);
        labInfo1.setText(Mess + Knob_Info[t].Suffix, juce::dontSendNotification);
        break;
    
      case 3: //R1.00 Special formatting for HIGH CUT. 
          if (audioProcessor.Setting[t] < 8000.0f)
          {
              Mess2 = std::to_string(int(audioProcessor.Setting[t]));
              labInfo1.setText(Mess2 + Knob_Info[t].Suffix, juce::sendNotification);
          }
          else
              labInfo1.setText("HC off", juce::sendNotification);
          break;
          
      case 4: //R1.00 Special formatting for Stereo / Mono.
          if (audioProcessor.Setting[t] < 1.0f)
              labInfo1.setText("Stereo", juce::sendNotification);
          else
              labInfo1.setText("Mono", juce::sendNotification);
          break;
    }
}

//R1.00 Override SLIDER control to capture setting changes.
void MakoBiteAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{       
    //R1.00 Catch the EQ BAND change here so we can update the UI and frequencies.
    if (slider == &sldKnob[e_EQ])
    {   
        audioProcessor.Setting[e_EQ] = float(sldKnob[e_EQ].getValue());
        Band_SetFilterValues(true);
        audioProcessor.SettingsChanged += 1;        
        KNOB_ShowValue(e_EQ);
        return; 
    }
       
    //R1.00 When a slider is adjusted, this func gets called. Capture the new edits
    //R1.00  and flag the processor when it needs to recalc things.
    //R1.00 Check which slider has been adjusted.
    for (int t = 0; t < Knob_Cnt; t++)
    {
        if (slider == &sldKnob[t])
        {   
            //R1.00 Update the actual processor variable being edited.
            audioProcessor.Setting[t] = float(sldKnob[t].getValue());

            //R1.00 We need to update settings in processor.
            //R1.00 Increment changed var to be sure every change gets made. Changed var is decremented in processor.
            audioProcessor.SettingsChanged += 1;

            //R1.00 Print the value to the UI.
            KNOB_ShowValue(t);

            //R1.00 We have captured the correct slider change, exit this function.
           return;
        }
    }
    
    return;
}

//R1.00 Define the user selected EQ mode.
//R1.00 These values must match the twin function in the PROCESSOR.
void MakoBiteAudioProcessorEditor::Band_SetFilterValues(bool ForcePaint)
{
    int EQ_Mode = int(audioProcessor.Setting[e_EQ]);
        
    switch (EQ_Mode)
    {
    default:
    {
        audioProcessor.Band1_Freq = 150.0f;
        audioProcessor.Band2_Freq = 300.0f;
        audioProcessor.Band3_Freq = 750.0f;
        audioProcessor.Band4_Freq = 1500.0f;
        audioProcessor.Band5_Freq = 3000.0f;
        audioProcessor.Band1_Q = .707f;
        audioProcessor.Band2_Q = 1.414f;
        audioProcessor.Band3_Q = 1.414f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = 1.414f;
        break;
    }
    case 1:
    {
        audioProcessor.Band1_Freq = 150.0f;
        audioProcessor.Band2_Freq = 450.0f;
        audioProcessor.Band3_Freq = 900.0f;
        audioProcessor.Band4_Freq = 1800.0f;
        audioProcessor.Band5_Freq = 3500.0f;
        audioProcessor.Band1_Q = .707f;
        audioProcessor.Band2_Q = 1.414f;
        audioProcessor.Band3_Q = 1.414f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = 1.414f;
        break;
    }
    case 2:
    {
        audioProcessor.Band1_Freq = 80.0f;
        audioProcessor.Band2_Freq = 220.0f;
        audioProcessor.Band3_Freq = 750.0f;
        audioProcessor.Band4_Freq = 2200.0f;
        audioProcessor.Band5_Freq = 6000.0f;
        audioProcessor.Band1_Q = .707f;
        audioProcessor.Band2_Q = 1.414f;
        audioProcessor.Band3_Q = 1.414f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = 1.414f;
        break;
    }
    case 3:
    {
        audioProcessor.Band1_Freq = 80.0f;
        audioProcessor.Band2_Freq = 350.0f;
        audioProcessor.Band3_Freq = 900.0f;
        audioProcessor.Band4_Freq = 1500.0f;
        audioProcessor.Band5_Freq = 3000.0f;
        audioProcessor.Band1_Q = .707f;
        audioProcessor.Band2_Q = 1.414f;
        audioProcessor.Band3_Q = 1.414f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = 1.414f;
        break;
    }
    case 4:
    {
        audioProcessor.Band1_Freq = 100.0f;
        audioProcessor.Band2_Freq = 400.0f;
        audioProcessor.Band3_Freq = 800.0f;
        audioProcessor.Band4_Freq = 1600.0f;
        audioProcessor.Band5_Freq = 3200.0f;
        audioProcessor.Band1_Q = .707f;
        audioProcessor.Band2_Q = 1.414f;
        audioProcessor.Band3_Q = 1.414f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = 1.414f;
        break;
    }
    case 5:
    {
        audioProcessor.Band1_Freq = 120.0f;
        audioProcessor.Band2_Freq = 330.0f;
        audioProcessor.Band3_Freq = 660.0f;
        audioProcessor.Band4_Freq = 1320.0f;
        audioProcessor.Band5_Freq = 2500.0f;
        audioProcessor.Band1_Q = .707f;
        audioProcessor.Band2_Q = 1.414f;
        audioProcessor.Band3_Q = .707f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = .707f;
        break;
    }
    case 6:
    {
        audioProcessor.Band1_Freq = 150.0f;
        audioProcessor.Band2_Freq = 500.0f;
        audioProcessor.Band3_Freq = 900.0f;
        audioProcessor.Band4_Freq = 1800.0f;
        audioProcessor.Band5_Freq = 5000.0f;
        audioProcessor.Band1_Q = 1.414f;
        audioProcessor.Band2_Q = .707f;
        audioProcessor.Band3_Q = 1.414f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = .707f;
        break;
    }
    case 7:
    {
        audioProcessor.Band1_Freq = 80.0f;
        audioProcessor.Band2_Freq = 300.0f;
        audioProcessor.Band3_Freq = 650.0f;
        audioProcessor.Band4_Freq = 1500.0f;
        audioProcessor.Band5_Freq = 4500.0f;
        audioProcessor.Band1_Q = 1.414f;
        audioProcessor.Band2_Q = .707f;
        audioProcessor.Band3_Q = 2.00f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = .707f;
        break;
    }
    case 8:
    {
        audioProcessor.Band1_Freq = 100.0f;
        audioProcessor.Band2_Freq = 400.0f;
        audioProcessor.Band3_Freq = 800.0f;
        audioProcessor.Band4_Freq = 1600.0f;
        audioProcessor.Band5_Freq = 4000.0f;
        audioProcessor.Band1_Q = .707f;
        audioProcessor.Band2_Q = .707f;
        audioProcessor.Band3_Q = 1.414f;
        audioProcessor.Band4_Q = 2.00f;
        audioProcessor.Band5_Q = .35f;
        break;
    }
    case 9:
    {
        audioProcessor.Band1_Freq = 80.0f;
        audioProcessor.Band2_Freq = 500.0f;
        audioProcessor.Band3_Freq = 1000.0f;
        audioProcessor.Band4_Freq = 2000.0f;
        audioProcessor.Band5_Freq = 5000.0f;
        audioProcessor.Band1_Q = .707f;
        audioProcessor.Band2_Q = 1.414f;
        audioProcessor.Band3_Q = .707f;
        audioProcessor.Band4_Q = .707f;
        audioProcessor.Band5_Q = .350f;
        break;
    }
    case 10:
    {
        audioProcessor.Band1_Freq = 80.0f;
        audioProcessor.Band2_Freq = 250.0f;
        audioProcessor.Band3_Freq = 750.0f;
        audioProcessor.Band4_Freq = 1800.0f;
        audioProcessor.Band5_Freq = 3600.0f;
        audioProcessor.Band1_Q = 2.000f;
        audioProcessor.Band2_Q = .707f;
        audioProcessor.Band3_Q = .35f;
        audioProcessor.Band4_Q = 1.414f;
        audioProcessor.Band5_Q = .350f;
        break;
    }

    }

    //R1.00 Create the UI label strings.    
    Knob_Info[e_EQ1].Name = std::to_string(int(audioProcessor.Band1_Freq));
    Knob_Info[e_EQ2].Name = std::to_string(int(audioProcessor.Band2_Freq));
    Knob_Info[e_EQ3].Name = std::to_string(int(audioProcessor.Band3_Freq));
    Knob_Info[e_EQ4].Name = std::to_string(int(audioProcessor.Band4_Freq));
    Knob_Info[e_EQ5].Name = std::to_string(int(audioProcessor.Band5_Freq));

    //R1.01 We changed some stuff so refresh the screen/UI.
    if (ForcePaint) repaint();

}
