#include "PluginProcessor.h"
#include "DelayUnit.h"

DelayUnit::DelayUnit():smoothedDelayTime(1000.0f){

}

void DelayUnit::prepareDelayUnit(int loopLengthInSeconds, double sampleRate, int samplesPerBlock)
{
	tape.setTapeLength(loopLengthInSeconds, sampleRate);
	tape.setUpFilter(sampleRate);

	// Allocate the expected max block size for our internal buffer, just one channel for now
	delayUnitInternalBuffer.setSize(1, samplesPerBlock);

	// Get read & write pointers
	delayUnitInternalBufferWritePointer = delayUnitInternalBuffer.getWritePointer(0);
	delayUnitInternalBufferReadPointer = delayUnitInternalBuffer.getReadPointer(0);

	smoothedDelayTime.reset(0);

	// Save the sampleRate internally
	this->sampleRate = sampleRate;

	// Set up delay in terms of samples
	actualDelayTimeInSamples = (size_t)juce::roundToInt(delayTime * (sampleRate / 1000));
}

void DelayUnit::process(float* inputBuffer, int numberOfSamples)
{
	previousDelayTime = delayTime;
	delayTime = (int)smoothedDelayTime.getNextValue();
	float feedback = this->feedback;

	// On change, set up our variables
	if (previousDelayTime != delayTime) {
		actualDelayTimeInSamples = (size_t)juce::roundToInt(delayTime * (sampleRate / 1000));
		int factor = 1;
		tapeWriteSpeed = (double)(delayTime*factor) / 1000;
		tapeReadSpeed = (double)(1 / (((double)delayTime*factor) / 1000));
	}

	// Put the current dry input block into the tape, at the correct speed
	tape.recordToTape((const float*)inputBuffer, numberOfSamples, tapeWriteSpeed);

	// Retroactively apply feedback to the dry input block
	tape.applyFeedback(feedback, actualDelayTimeInSamples, tapeReadSpeed, numberOfSamples);

	// Retrieve a delayed block from our delay unit, to be combined with the input signal & sent to output
	tape.getInterpolatedDelayedBufferFromTape(delayUnitInternalBufferWritePointer, (((double)actualDelayTimeInSamples*tapeReadSpeed)) + (numberOfSamples*tapeReadSpeed), numberOfSamples, tapeReadSpeed);


	// Get appropriate samples out of delay buffer & add them to signal
	for (size_t i = 0; i < numberOfSamples; ++i) {
		float outputSample = inputBuffer[i] + (wetness * delayUnitInternalBuffer.getSample(0, i));
		inputBuffer[i] = outputSample;
	}
}

void DelayUnit::changeDelayTimeBbd(int delayTime) {
	this->smoothedDelayTime.setTargetValue(delayTime);
}

void DelayUnit::changeFeedbackLevel(int level)
{
	this->feedback = (float)level/100;
}

void DelayUnit::enableDebug(bool setting) {
	tape.debugMode = setting;
}