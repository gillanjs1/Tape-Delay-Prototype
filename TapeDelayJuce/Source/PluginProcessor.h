#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "TapeLoop.h"
#include "TapeTest.h"
#include "DelayUnit.h"


//==============================================================================
/**
*/
class TapeDelayJuceAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    TapeDelayJuceAudioProcessor();
    ~TapeDelayJuceAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

	void changeDelaySpeed(int value);
	void changeFeedbackLevel(int value);

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:

	DelayUnit delayUnit;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeDelayJuceAudioProcessor)
};
