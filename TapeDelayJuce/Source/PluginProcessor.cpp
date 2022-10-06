#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapeDelayJuceAudioProcessor::TapeDelayJuceAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	bool runTests = false;
	#ifdef JUCE_DEBUG
		// default to running tests in debug builds; disable on cmdline 
		runTests = true;
	#else
		// in release mode, default to not running tests.
		runTests = false;
	#endif

	if (runTests)
	{
		UnitTestRunner testRunner;
		testRunner.runAllTests();
	}
}

TapeDelayJuceAudioProcessor::~TapeDelayJuceAudioProcessor()
{
}

//==============================================================================
const String TapeDelayJuceAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapeDelayJuceAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TapeDelayJuceAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TapeDelayJuceAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TapeDelayJuceAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TapeDelayJuceAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TapeDelayJuceAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapeDelayJuceAudioProcessor::setCurrentProgram (int index)
{
}

const String TapeDelayJuceAudioProcessor::getProgramName (int index)
{
    return {};
}

void TapeDelayJuceAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void TapeDelayJuceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

	// If adjusting this, make sure to immediately test with max & min delay times
	delayUnit.prepareDelayUnit(15, sampleRate, samplesPerBlock);
	//delayUnit.changeDelayTimeBbd(517);
}

void TapeDelayJuceAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TapeDelayJuceAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void TapeDelayJuceAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

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
		auto* channelData = buffer.getWritePointer(channel);

        // ..do something to the data...
		// Just recording left channel for now
		if (channel == 0) {
			delayUnit.process(channelData, buffer.getNumSamples());
		} else {
		const float* otherChannelData = buffer.getReadPointer(0);
		std::copy(otherChannelData, otherChannelData + buffer.getNumSamples(), channelData);
		}

    }
}

void TapeDelayJuceAudioProcessor::changeDelaySpeed(int value) {
	delayUnit.changeDelayTimeBbd(value);
}

void TapeDelayJuceAudioProcessor::changeFeedbackLevel(int value) {
	delayUnit.changeFeedbackLevel(value);
}

//==============================================================================
bool TapeDelayJuceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* TapeDelayJuceAudioProcessor::createEditor()
{
    return new TapeDelayJuceAudioProcessorEditor (*this);
}

//==============================================================================
void TapeDelayJuceAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TapeDelayJuceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeDelayJuceAudioProcessor();
}
