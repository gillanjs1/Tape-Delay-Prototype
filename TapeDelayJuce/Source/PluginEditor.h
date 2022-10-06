#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TapeDelayJuceAudioProcessorEditor  : public AudioProcessorEditor,
										   private Slider::Listener
{
public:
    TapeDelayJuceAudioProcessorEditor (TapeDelayJuceAudioProcessor&);
    ~TapeDelayJuceAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TapeDelayJuceAudioProcessor& processor;

	void sliderValueChanged(Slider* slider) override;
	Slider delayTimeSlider;
	Label delayTimeSliderLabel;

	Slider feedbackSlider;
	Label feedbackSliderLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeDelayJuceAudioProcessorEditor)
};
