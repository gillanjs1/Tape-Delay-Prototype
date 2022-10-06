#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapeDelayJuceAudioProcessorEditor::TapeDelayJuceAudioProcessorEditor (TapeDelayJuceAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1000, 300);
	addAndMakeVisible(&delayTimeSlider);

	// these define the parameters of our slider object
	delayTimeSlider.setSliderStyle(Slider::LinearHorizontal);
	delayTimeSlider.setRange(25.0, 1000.0, 1.0);
	delayTimeSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
	delayTimeSlider.setPopupDisplayEnabled(true, false, this);
	delayTimeSlider.setTextValueSuffix(" ms");
	delayTimeSlider.setValue(1.0);

	// this function adds the slider to the editor
	delayTimeSlider.addListener(this);

	addAndMakeVisible(delayTimeSliderLabel);
	delayTimeSliderLabel.setText("Delay Time", dontSendNotification);
	delayTimeSliderLabel.attachToComponent(&delayTimeSlider, false);
	addAndMakeVisible(&feedbackSlider);

	// these define the parameters of our slider object
	feedbackSlider.setSliderStyle(Slider::LinearHorizontal);
	feedbackSlider.setRange(0.0, 100.0, 1.0);
	feedbackSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
	feedbackSlider.setPopupDisplayEnabled(true, false, this);
	feedbackSlider.setTextValueSuffix(" %");
	feedbackSlider.setValue(1.0);

	// this function adds the slider to the editor
	feedbackSlider.addListener(this);

	addAndMakeVisible(feedbackSliderLabel);
	feedbackSliderLabel.setText("Feedback", dontSendNotification);
	feedbackSliderLabel.attachToComponent(&feedbackSlider, false);
}

TapeDelayJuceAudioProcessorEditor::~TapeDelayJuceAudioProcessorEditor()
{
}

void TapeDelayJuceAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
	if (slider == &delayTimeSlider) {
		processor.changeDelaySpeed((int)slider->getValue());
	}

	if (slider == &feedbackSlider) {
		processor.changeFeedbackLevel((int)slider->getValue());
	}
}


//==============================================================================
void TapeDelayJuceAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void TapeDelayJuceAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	delayTimeSlider.setBounds(10, 110, getWidth() - 20, 20);
	feedbackSlider.setBounds(10, 150, getWidth() - 20, 20);
}
