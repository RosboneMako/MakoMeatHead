/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "cmath"              //R1.00 Added library.

//==============================================================================
MakoBiteAudioProcessor::MakoBiteAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
    ),
    
    //R1.00 Define our VALUE TREE parameter variables. Min val, Max Val, default Val.
    parameters(*this, nullptr, "PARAMETERS", 
      {
        std::make_unique<juce::AudioParameterFloat>("gain","Gain", .0f, 1.0f, .24f),
        std::make_unique<juce::AudioParameterFloat>("ngate","Noise Gate", .0f, 1.0f, .0f),
        std::make_unique<juce::AudioParameterFloat>("drive","Drive", .0f, 1.0f, .24f),
        std::make_unique<juce::AudioParameterFloat>("midq","Mid Q", .1f, 3.0f, 1.0f),
        std::make_unique<juce::AudioParameterInt>("eq","EQ Band", 0.0f, 10.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq1","EQ 1", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq2","EQ 2", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq3","EQ 3", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq4","EQ 4", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("eq5","EQ 5", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterInt>("ir","IR Model", 1, 5, 1),
        std::make_unique<juce::AudioParameterInt>("iron","IR On", 0, 1, 1),
        std::make_unique<juce::AudioParameterInt>("mono","Mono", 0, 1, 1),
        std::make_unique<juce::AudioParameterFloat>("thump","Thump", .0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterInt>("highcut","High Cut", 2000, 8000, 8000),
      }
    )   

#endif
{   
}

MakoBiteAudioProcessor::~MakoBiteAudioProcessor()
{
}

//==============================================================================
const juce::String MakoBiteAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MakoBiteAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MakoBiteAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MakoBiteAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MakoBiteAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MakoBiteAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MakoBiteAudioProcessor::getProgramName (int index)
{
    return {};
}

void MakoBiteAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MakoBiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //R1.00 Get our Sample Rate for filter calculations.
    SampleRate = MakoBiteAudioProcessor::getSampleRate();
    if (SampleRate < 21000) SampleRate = 48000;
    if (192000 < SampleRate) SampleRate = 48000;

    //R1.00 PreCalc some values for Thining based on Sample Rate.
    Thin_Rate_High = .99f;
    Thin_Rate_Low = .01f;
    if (48001.0f < SampleRate)
    {
        Thin_Rate_High = .995f;
        Thin_Rate_Low = .005f;
    }
    if (96001.0f < SampleRate)
    {
        Thin_Rate_High = .9975f;
        Thin_Rate_Low = .0025f;
    }

    //R1.00 PreCalc some values for Signal Averaging based on Sample Rate.
    AVG_Rate_High = .995f;
    AVG_Rate_Low = .005f;
    if (48001.0f < SampleRate)
    {
        AVG_Rate_High = .9975f;
        AVG_Rate_Low = .0025f;
    }
    if (96001.0f < SampleRate)
    {
        AVG_Rate_High = .99875f;
        AVG_Rate_Low = .00125f;
    }

    //R1.00 Calculate some rough decay subtraction values for peak tracking (compress,autowah,etc). 
    Release_100mS = (1.0f / .100f) * (1.0f / SampleRate);
    Release_200mS = (1.0f / .200f) * (1.0f / SampleRate);
    Release_300mS = (1.0f / .300f) * (1.0f / SampleRate);
    Release_400mS = (1.0f / .400f) * (1.0f / SampleRate);
    Release_500mS = (1.0f / .500f) * (1.0f / SampleRate);

    //R1.00 Calc our filters.
    Filter_HP_Coeffs(150.0f, &makoF_LowCut);                    //R1.00 Increase this if bass is too mushy. 180,200, etc
    Filter_LP_Coeffs(150.0f, &makoF_Thump);
    Filter_BP_Coeffs(9.0f, 1800.0f, .35f, &makoF_HighBoost);
    Mako_Settings_Update(true);

    //R1.00 Create our initial IR and set a volume factor for each stored IR.  
    IR_VolAdjustVals[1] = .29f;
    IR_VolAdjustVals[2] = .19f;
    IR_VolAdjustVals[3] = .25f;
    IR_VolAdjustVals[4] = .27f;
    IR_VolAdjustVals[5] = .25f;
    Mako_IR_Set();
}

void MakoBiteAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MakoBiteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MakoBiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //R1.00 Our defined variables.
    float tS;  //R1.00 Temporary Sample.

    //R1.00 Handle any changes to our Paramters.
    //R1.00 Handle any settings changes made in Editor. 
    if (0 < SettingsChanged) Mako_Settings_Update(false);

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        //R1.00 Process the AUDIO buffer data.
        if (Pedal_Mono && (channel == 1))
        {
            auto* channel0Data = buffer.getWritePointer(0);

            //R1.00 FORCE MONO - Put CHANNEL 0 data in CHANNEL 1.
            for (int samp = 0; samp < buffer.getNumSamples(); samp++) channelData[samp] = channel0Data[samp];
        }
        else
        {
            // ..do something to the data...
            for (int samp = 0; samp < buffer.getNumSamples(); samp++)
            {
                //R1.00 Get the current sample and put it in tS. 
                tS = buffer.getSample(channel, samp);

                //R1.00 Noise gate.
                if (0.0f < Setting[e_NGate]) tS = Mako_FX_NoiseGate(tS, channel);
                
                //R1.00 Apply our Distortion to the sample. 
                tS = Mako_FX_AmpSim(tS, channel);

                //R1.00 Impulse Response (IR). 0 = Off.
                if (0.0f < Setting[e_IR]) tS = Mako_CabSim(tS, channel);

                //R1.00 See if we are clipping and flag the editor to show UI notice.
                Mako_CheckForClipping(tS);

                //R1.00 Write our modified sample back into the sample buffer.
                channelData[samp] = tS;
            }
        }
    }
}

//==============================================================================
bool MakoBiteAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MakoBiteAudioProcessor::createEditor()
{
    return new MakoBiteAudioProcessorEditor (*this);
}


//==============================================================================
void MakoBiteAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    //R1.00 Save our parameters to file/DAW.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
   
}

void MakoBiteAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    //R1.00 Read our parameters from file/DAW.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

    //R1.00 Force our variables to get updated.
    Setting[e_Gain] = makoGetParmValue_float("gain");
    Setting[e_NGate] = makoGetParmValue_float("ngate");
    Setting[e_Drive] = makoGetParmValue_float("drive");
    Setting[e_MidQ] = makoGetParmValue_float("midq");
    Setting[e_EQ] = makoGetParmValue_float("eq");
    Setting[e_EQ1] = makoGetParmValue_float("eq1");
    Setting[e_EQ2] = makoGetParmValue_float("eq2");
    Setting[e_EQ3] = makoGetParmValue_float("eq3");
    Setting[e_EQ4] = makoGetParmValue_float("eq4");
    Setting[e_EQ5] = makoGetParmValue_float("eq5");
    Setting[e_IR] = makoGetParmValue_float("ir");
    Setting[e_Mono] = makoGetParmValue_float("mono");
    Setting[e_Thump] = makoGetParmValue_float("thump");
    Setting[e_HighCut] = makoGetParmValue_float("highcut");
}


//R1.00 Parameter reading helper function.
int MakoBiteAudioProcessor::makoGetParmValue_int(juce::String Pstring)
{
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return int(parm->load());
    else
        return 0;
}

//R1.00 Parameter reading helper function.
float MakoBiteAudioProcessor::makoGetParmValue_float(juce::String Pstring)
{
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return float(parm->load());
    else
        return 0.0f;
}

//R1.00 Volume envelope based on average Signal volume.
float MakoBiteAudioProcessor::Mako_FX_NoiseGate(float tSample, int channel)
{
    //R1.00 Track our Input Signal Average (Absolute vals).
    Signal_AVG[channel] = (Signal_AVG[channel] * AVG_Rate_High) + (abs(tSample) * AVG_Rate_Low);
    
    //R1.00 Create a volume envelope based on Signal Average.
    Pedal_NGate_Fac[channel] = Signal_AVG[channel] * 10000.0f * (1.1f - Setting[e_NGate]);

    //R1.00 Dont amplify the sound, just reduce when necessary.
    if (1.0f < Pedal_NGate_Fac[channel]) Pedal_NGate_Fac[channel] = 1.0f;

    return tSample * Pedal_NGate_Fac[channel];
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MakoBiteAudioProcessor();
}

//R1.00 Apply filter to a sample.
float MakoBiteAudioProcessor::Filter_Calc_BiQuad(float tSample, int channel, tp_filter* fn)
{
    float tS = tSample;

    fn->xn0[channel] = tS;
    tS = fn->a0 * fn->xn0[channel] + fn->a1 * fn->xn1[channel] + fn->a2 * fn->xn2[channel] - fn->b1 * fn->yn1[channel] - fn->b2 * fn->yn2[channel];
    fn->xn2[channel] = fn->xn1[channel]; fn->xn1[channel] = fn->xn0[channel]; fn->yn2[channel] = fn->yn1[channel]; fn->yn1[channel] = tS;

    return tS;
}

//R1.00 Second order parametric/peaking boost filter with constant-Q
void MakoBiteAudioProcessor::Filter_BP_Coeffs(float Gain_dB, float Fc, float Q, tp_filter* fn)
{    
    float K = pi2 * (Fc * .5f) / SampleRate;
    float K2 = K * K;
    float V0 = pow(10.0, Gain_dB / 20.0);

    float a = 1.0f + (V0 * K) / Q + K2;
    float b = 2.0f * (K2 - 1.0f);
    float g = 1.0f - (V0 * K) / Q + K2;
    float d = 1.0f - K / Q + K2;
    float dd = 1.0f / (1.0f + K / Q + K2);

    fn->a0 = a * dd;
    fn->a1 = b * dd;
    fn->a2 = g * dd;
    fn->b1 = b * dd;
    fn->b2 = d * dd;
    fn->c0 = 1.0f;
    fn->d0 = 0.0f;
}

//R1.00 Second order LOW PASS filter. 
void MakoBiteAudioProcessor::Filter_LP_Coeffs(float fc, tp_filter* fn)
{    
    float c = 1.0f / (tanf(pi * fc / SampleRate));
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = 2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * (1.0f - (c * c));
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}

//F1.00 Second order butterworth High Pass.
void MakoBiteAudioProcessor::Filter_HP_Coeffs(float fc, tp_filter* fn)
{    
    float c = tanf(pi * fc / SampleRate);
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = -2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * ((c * c) - 1.0f);
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}


float MakoBiteAudioProcessor::Mako_FX_AmpSim(float tSample, int channel)
{   
    float tS = tSample;
    float tS2;
    float tS_Thump;
    float tS_Peak;
    float tComp;
    float tS_Enh;

    //R1.00 Apply our Tube Thinner. Reduce volume during heavy distortion to reduce ghost notes.
    tS *= Pedal_ThinAdj[channel];

    //R1.00 Add some rectified signal.
    tS = tS + (.05f * abs(tS));

    //R1.00 HP - Low Cut.
    tS = Filter_Calc_BiQuad(tS, channel, &makoF_LowCut);

    //R1.00 Calc our High Freq enhance filter. 
    tS = Filter_Calc_BiQuad(tS, channel, &makoF_HighBoost);
    tS = tanhf(tS * (2.0f * Setting[e_MidQ]));
            
    //R1.00 BP - Mid boost.
    tS = Filter_Calc_BiQuad(tS, channel, &makoF_MidBoost);
    
    //R1.00 Apply Drive.
    tS = tS * (1.0f + (Setting[e_Drive] * 750.0f));
    
    //R1.00 Apply angled clipping curve.
    tS2 = tS;                                             //R1.00 Track Pos/Neg.
    tS = abs(tS);                                         //R1.00 Get positive value for math to work.
    if (0.75f < tS) tS = 0.75f + (tS - 0.75f) * 0.35f;
    tS_Peak = abs(tS);
    if (tS2 < 0.0f) tS = -tS;                             //R1.00 Restore polarity.
    
    //R1.00 Clip.
    if (.9999f < tS) tS = .9999f;
    if (tS < -.9999f) tS = -.9999f;
        
    //*******************************************
    // TUBE THINNING / COMPRESSION
    //*******************************************
    //R1.00 Apply Compression based on how far past 1.0f we are going.
    //R1.00 Create a temp compression value.
    if (tS_Peak < 1.0f)
        tComp = 1.0f;
    else
        tComp = 1 / tS_Peak;
    if (tComp < .01f) tComp = .01f;

    //R1.00 Average the new Compression value into the actual compression value. 
    Pedal_ThinAdj[channel] = (Pedal_ThinAdj[channel] * Thin_Rate_High) + (tComp * Thin_Rate_Low);

    //R1.00 Thump
    tS_Thump = Filter_Calc_BiQuad(tS, channel, &makoF_Thump);
    tS = tS + tanhf(tS_Thump * Setting[e_Thump] * 4.0f);

    //R1.00 Apply EQ. Try to not to calc, if not needed, to save CPU cycles.    
    if (Setting[e_EQ1] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band1);
    if (Setting[e_EQ2] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band2);
    if (Setting[e_EQ3] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band3);
    if (Setting[e_EQ4] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band4);
    if (Setting[e_EQ5] != .0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Band5);

    //R1.00 LP - Fizz filter.
    if (Setting[e_HighCut] < 8000.0f) tS = Filter_Calc_BiQuad(tS, channel, &makoF_HighCut);
        
    //R1.00 Volume/Gain adjust. Reduction only.
    tS *= Setting[e_Gain];

    return tS;
}

//R1.00 We do changes here so we know the vars are not in use while we change them.
//R1.00 EDITOR sets SETTING flags and we make changes here.
void MakoBiteAudioProcessor::Mako_Settings_Update(bool ForceAll)
{    
    bool Force = ForceAll;

    //R1.00 Mid boost.
    Filter_BP_Coeffs(18.0f, 650.0f, Setting[e_MidQ], &makoF_MidBoost);

    //R1.00 Low Pass. 
    Filter_LP_Coeffs(Setting[e_HighCut], &makoF_HighCut);       

    //R1.00 Update our EQ Filters.
    Band_SetFilterValues();
    Filter_BP_Coeffs(Setting[e_EQ1], Band1_Freq, Band1_Q, &makoF_Band1);
    Filter_BP_Coeffs(Setting[e_EQ2], Band2_Freq, Band2_Q, &makoF_Band2);
    Filter_BP_Coeffs(Setting[e_EQ3], Band3_Freq, Band3_Q, &makoF_Band3);
    Filter_BP_Coeffs(Setting[e_EQ4], Band4_Freq, Band4_Q, &makoF_Band4);
    Filter_BP_Coeffs(Setting[e_EQ5], Band5_Freq, Band5_Q, &makoF_Band5);    

    //R1.00 Additional ENHANCE filters.
    if ((Setting[e_IR] != Setting_Last[e_IR]) || Force)
    {
        Setting_Last[e_IR] = Setting[e_IR];
        Mako_IR_Set();
    }

    //R1.00 RESET out settings flags.
    SettingsType = 0;
    SettingsChanged = false;
}

float MakoBiteAudioProcessor::Mako_CabSim(float tSample, int channel)
{
    int T1;
    float V = 0.0f;
    
    //R1.00 Get the next buffer position Index and store the new incoming Sample.
    T1 = IRB_Idx[channel];
    IRB[channel][T1] = tSample;

    //R1.00 Calculate the IR response by multiplying every IR sample by our audio buffer samples.
    //R1.00 Effectively it is a DELAY(comb filter) pedal with 1024 repeats in a very short time.
    //R1.00 The repeats will add and zero out signals due to phase which creates an EQ filter.
    //R1.00 The IR acts as both a delay and filter combined.
    //R1.00 For absolute best sound an IR should be 2048 samples. But the IR calc is very heavy on CPU usage.
    //R1.00 Using 1024 here for good sound and less CPU usage. Could add slider and make it even shorter.
    for (int t = 0; t < 1024; t++)
    {
        V += (IR_Final[t] * IRB[channel][T1]);

        //R1.00 Increment index, mask off bits past 1023 to keep our index between 0-1023.
        //R1.00 IR length must be a power of 2 for masking to work 1024(3FF), 2048(7FF) are standard sizes.
        T1 = (T1 + 1) & 0x3FF;
    }
    
    //R1.00 Decrement our buffer index and loop around at 0. 
    IRB_Idx[channel]--;
    if (IRB_Idx[channel] < 0) IRB_Idx[channel] = 1023;

    //R1.00 We usually gain volume here so reduce it.
    return V * IR_Final_VolAdjust;
}

void MakoBiteAudioProcessor::Mako_IR_Set()
{
    int IR_Model = int(Setting[e_IR]);

    //R1.00 Put one of the preset IRs into the actual IR used for processing.
    //R1.00 We could just use an index into the current array, but this lets us do
    //R1.00 other modifications to the IRs as well. Like resampling, etc.
    switch (IR_Model)
    {
        case 1: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_01[t]; break;
        case 2: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_02[t]; break;
        case 3: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_03[t]; break;
        case 4: for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_04[t]; break;
        default:for (int t = 0; t < 1024; t++) IR_Final[t] = IR_Stored_05[t]; break;
    }

    //R1.00 These volumes are estimated values defined in Prepare to play.
    //R1.00 Could do complicated math to get better values. Close enough for us.
    IR_Final_VolAdjust = IR_VolAdjustVals[IR_Model];

    return;
}

void MakoBiteAudioProcessor::Mako_CheckForClipping(float tSample)
{
    //R1.00 Clip the signal to just below -1/1 so the audio engine does not crash. 
    //R1.00 Need a var here to let user know they are clipping. This is read in the
    //R1.00 EDITOR Timer code to show clipping on the UI.
    if (tSample < -.9999f)
    {
        tSample = -.9999f;
        AudioIsClipping = true;
    }
    if (.9999f < tSample)
    {
        tSample = .9999f;
        AudioIsClipping = true;
    }
}

//R1.00 This function is used so our filters are setup correctly on load.
//R1.00 Must match the same func in PluginEditor.cpp.
void MakoBiteAudioProcessor::Band_SetFilterValues()
{
    int EQ_Mode = int(Setting[e_EQ]);

    //R1.01 Define the user selected EQ mode.
    switch (EQ_Mode)
    {
    default:
    {
        Band1_Freq = 150.0f;
        Band2_Freq = 300.0f;
        Band3_Freq = 750.0f;
        Band4_Freq = 1500.0f;
        Band5_Freq = 3000.0f;
        Band1_Q = .707f;
        Band2_Q = 1.414f;
        Band3_Q = 1.414f;
        Band4_Q = 1.414f;
        Band5_Q = 1.414f;
        break;
    }
    case 1:
    {
        Band1_Freq = 150.0f;
        Band2_Freq = 450.0f;
        Band3_Freq = 900.0f;
        Band4_Freq = 1800.0f;
        Band5_Freq = 3500.0f;
        Band1_Q = .707f;
        Band2_Q = 1.414f;
        Band3_Q = 1.414f;
        Band4_Q = 1.414f;
        Band5_Q = 1.414f;
        break;
    }
    case 2:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 220.0f;
        Band3_Freq = 750.0f;
        Band4_Freq = 2200.0f;
        Band5_Freq = 6000.0f;
        Band1_Q = .707f;
        Band2_Q = 1.414f;
        Band3_Q = 1.414f;
        Band4_Q = 1.414f;
        Band5_Q = 1.414f;
        break;
    }
    case 3:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 350.0f;
        Band3_Freq = 900.0f;
        Band4_Freq = 1500.0f;
        Band5_Freq = 3000.0f;
        Band1_Q = .707f;
        Band2_Q = 1.414f;
        Band3_Q = 1.414f;
        Band4_Q = 1.414f;
        Band5_Q = 1.414f;
        break;
    }
    case 4:
    {
        Band1_Freq = 100.0f;
        Band2_Freq = 400.0f;
        Band3_Freq = 800.0f;
        Band4_Freq = 1600.0f;
        Band5_Freq = 3200.0f;
        Band1_Q = .707f;
        Band2_Q = 1.414f;
        Band3_Q = 1.414f;
        Band4_Q = 1.414f;
        Band5_Q = 1.414f;
        break;
    }
    case 5:
    {
        Band1_Freq = 120.0f;
        Band2_Freq = 330.0f;
        Band3_Freq = 660.0f;
        Band4_Freq = 1320.0f;
        Band5_Freq = 2500.0f;
        Band1_Q = .707f;
        Band2_Q = 1.414f;
        Band3_Q = .707f;
        Band4_Q = 1.414f;
        Band5_Q = .707f;
        break;
    }
    case 6:
    {
        Band1_Freq = 150.0f;
        Band2_Freq = 500.0f;
        Band3_Freq = 900.0f;
        Band4_Freq = 1800.0f;
        Band5_Freq = 5000.0f;
        Band1_Q = 1.414f;
        Band2_Q = .707f;
        Band3_Q = 1.414f;
        Band4_Q = 1.414f;
        Band5_Q = .707f;
        break;
    }
    case 7:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 300.0f;
        Band3_Freq = 650.0f;
        Band4_Freq = 1500.0f;
        Band5_Freq = 4500.0f;
        Band1_Q = 1.414f;
        Band2_Q = .707f;
        Band3_Q = .35f;
        Band4_Q = 1.414f;
        Band5_Q = .707f;
        break;
    }
    case 8:
    {
        Band1_Freq = 100.0f;
        Band2_Freq = 400.0f;
        Band3_Freq = 800.0f;
        Band4_Freq = 1600.0f;
        Band5_Freq = 4000.0f;
        Band1_Q = .707f;
        Band2_Q = .707f;
        Band3_Q = 1.414f;
        Band4_Q = 2.00f;
        Band5_Q = .35f;
        break;
    }
    case 9:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 500.0f;
        Band3_Freq = 1000.0f;
        Band4_Freq = 2000.0f;
        Band5_Freq = 5000.0f;
        Band1_Q = .707f;
        Band2_Q = 1.414f;
        Band3_Q = .707f;
        Band4_Q = .707f;
        Band5_Q = .350f;
        break;
    }
    case 10:
    {
        Band1_Freq = 80.0f;
        Band2_Freq = 250.0f;
        Band3_Freq = 750.0f;
        Band4_Freq = 1800.0f;
        Band5_Freq = 3600.0f;
        Band1_Q = 2.000f;
        Band2_Q = .707f;
        Band3_Q = .35f;
        Band4_Q = 1.414f;
        Band5_Q = .350f;
        break;
    }

    }

}
