#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
*/

class DelayUnit
{
public:
	DelayUnit();
	void prepareDelayUnit(int loopLengthInSeconds, double sampleRate, int samplesPerBlock);
	void process(float* inputBuffer, int numberOfSamples);

	void changeDelayTimeBbd(int delayTime);
	void changeFeedbackLevel(int level);

	void enableDebug(bool setting);

private:
	// Just the one tape loop for now, need to move to two
	TapeLoop tape;
	AudioBuffer<float> delayUnitInternalBuffer;
	double sampleRate;
	float feedback{ 0.5f };
	int delayTime{ 1000 };
	int previousDelayTime{ 1000 };
	int actualDelayTimeInSamples{ 0 };
	float wetness{ 1.0f };
	double tapeReadSpeed{ 1.0 };
	double tapeWriteSpeed{ 1.0 };
	int targetDelayTime{ 1000 };

	// TODO remove any unused smoothedDelayTime
	SmoothedValue<float, ValueSmoothingTypes::Linear> smoothedDelayTime;
	//SmoothedValue<float, ValueSmoothingTypes::Multiplicative> smoothedDelayTime;
	//dsp::LogRampedValue< float > smoothedDelayTime;
	bool changeAllowed{ true };
	bool testSwitchingDelayTime{ false };
	float* delayUnitInternalBufferWritePointer;
	const float* delayUnitInternalBufferReadPointer;
};