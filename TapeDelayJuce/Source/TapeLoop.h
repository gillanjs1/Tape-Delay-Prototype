#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/
class TapeLoop
{
public:
	bool debugMode{false};

	// Constructor
	TapeLoop();
	void setTapeLength(int loopLengthInSeconds, double sampleRate);
	void setUpFilter(double sampleRate);
	void toggleFilter();
	void recordToTape(const float * inputPointer, int numOfSamplesToRecord);
	void recordToTape(const float * inputPointer, int numOfSamplesToRecord, double speed);
	void applyFeedback(float feedback, int originalDelayInSamples, double readSpeed, int bufferLength);
	float getDelayedSampleFromTape(int delayInSamples);
	float getInterpolatedDelayedSampleFromTape(double delayInSamples);
	float linearlyInterpolateBetweenSamples(float sample1, float sample2, double delayInSamples);
	void getInterpolatedDelayedBufferFromTape(float * inputPointer, double delayInSamples, int numberOfSamplesToGet, double tapeSpeed);
	void getDelayedBufferFromTape(float * inputPointer, int delayInSamples, int numberOfSamplesToGet);
	void getDelayedBufferFromTape(float * inputPointer, int delayInSamples, int numberOfSamplesToGet, double tapeSpeed);
	void getFromTape(float * outputPointer, int numOfSamplesToGet);
	void getFromTape(float * outputPointer, int numOfSamplesToGet, double speed);
	std::vector<float> getTapeLoopData();
	void resetTapeLoopDataReadPointer();

private:

	// This is a vector containing the delay buffer data
	std::vector<float> tapeLoopData;
	float *tapeLoopDataWritePointer;
	float *tapeLoopDataReadPointer;
	const float *tapeLoopDataStartPointer;
	const float *tapeLoopDataEndPointer;
	std::vector<float> catmullResults;
	std::vector<float> resultsVector;
	std::vector<float> retrivedBuffer;
	IIRFilter filter;

	// Disabling filter by default
	bool currentlyFiltering{ false };
	CatmullRomInterpolator recordCatmull;
	CatmullRomInterpolator readCatmull;
	void readDirectlyFromTape(float * outputPointer, int numOfSamplesToGet);
	void recordDirectlyToTape(const float * inputPointer, int numOfSamplesToRecord);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeLoop)
};
