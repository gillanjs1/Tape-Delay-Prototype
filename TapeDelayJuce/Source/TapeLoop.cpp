#include "TapeLoop.h"

// Constructor
TapeLoop::TapeLoop() {
}

void TapeLoop::setTapeLength(int loopLengthInSeconds, double sampleRate) {
	tapeLoopData.resize(loopLengthInSeconds * sampleRate);
	resultsVector.resize(480000);
	catmullResults.resize(480000);
	retrivedBuffer.resize(480000);

	// Set a write pointer to the first element
	tapeLoopDataWritePointer = &tapeLoopData[0];
	tapeLoopDataStartPointer = &tapeLoopData[0];
	tapeLoopDataReadPointer = &tapeLoopData[0];

	tapeLoopDataEndPointer = &tapeLoopData[tapeLoopData.size() - 1];
}

void TapeLoop::setUpFilter(double sampleRate) {

	// Sample rate * 0.5 is the highest you can go
	filter.setCoefficients(IIRCoefficients::makeLowPass(sampleRate, (sampleRate * 0.5) - 10000, 0.1));
}

void TapeLoop::toggleFilter() {
	currentlyFiltering = !currentlyFiltering;
}

void TapeLoop::recordToTape(const float* inputPointer, int numOfSamplesToRecord) {
	recordToTape(inputPointer, numOfSamplesToRecord, 1.0);
}

void TapeLoop::recordToTape(const float* inputPointer, int numOfSamplesToRecord, double speed) {
	int actualNumberOfSamplesToRecord = 0;
	actualNumberOfSamplesToRecord = std::floor((double)numOfSamplesToRecord / speed);
	float *catmullResultsPointer = &catmullResults[0];

	// Go and re-set the actual, accurate speed we want to use, now we know exactly how many samples we're recording
	double actualSpeed = (double)numOfSamplesToRecord / (double)actualNumberOfSamplesToRecord;

	recordCatmull.process(actualSpeed, inputPointer, catmullResultsPointer, actualNumberOfSamplesToRecord, numOfSamplesToRecord, false);

	if (currentlyFiltering) {
		filter.processSamples(catmullResultsPointer, actualNumberOfSamplesToRecord);
	}

	recordDirectlyToTape(catmullResultsPointer, actualNumberOfSamplesToRecord);
}

// Goes through and retroactively applies feed to an area already in the tape
void TapeLoop::applyFeedback(float feedback, int originalDelayInSamples, double readSpeed, int bufferLength) {

	int numOfSamplesToGet = (int)(bufferLength * readSpeed);

	float* rBuffer = &retrivedBuffer[0];
	getDelayedBufferFromTape(rBuffer, numOfSamplesToGet, numOfSamplesToGet);

	for (size_t i = 0; i < numOfSamplesToGet; ++i) {
		float delayedSample = getInterpolatedDelayedSampleFromTape(((numOfSamplesToGet)+originalDelayInSamples * readSpeed) - i);
		if (!debugMode) {
			retrivedBuffer[i] = std::tanh(retrivedBuffer[i] + feedback * delayedSample);
		}
		else {
			retrivedBuffer[i] = retrivedBuffer[i] + feedback * delayedSample;
		}
	}

	float* writePointerCopy = tapeLoopDataWritePointer;
	int currentIndex = tapeLoopDataWritePointer - tapeLoopDataStartPointer;
	int indexToRetrieve = currentIndex - (int)(bufferLength * readSpeed);
	if (indexToRetrieve >= 0) {
		tapeLoopDataWritePointer = &tapeLoopData[indexToRetrieve];
	}
	else {
		tapeLoopDataWritePointer = &tapeLoopData[tapeLoopData.size() + indexToRetrieve];
	}

	recordDirectlyToTape(rBuffer, bufferLength * readSpeed);
	tapeLoopDataWritePointer = writePointerCopy;
}

float TapeLoop::getDelayedSampleFromTape(int delayInSamples) {
	int currentIndex = tapeLoopDataWritePointer - tapeLoopDataStartPointer;
	int indexToRetrieve = currentIndex - delayInSamples;
	if (indexToRetrieve >= 0) {
		return tapeLoopData[indexToRetrieve];
	}
	else {
		return tapeLoopData[tapeLoopData.size() + indexToRetrieve];
	}
}

float TapeLoop::getInterpolatedDelayedSampleFromTape(double delayInSamples) {
	int flooredDelayInSamples = std::floor(delayInSamples);

	// If it's already at a whole number, continue as always using regular non-fractional method
	if (flooredDelayInSamples == delayInSamples) {
		return getDelayedSampleFromTape(int(delayInSamples));
	}
	int currentIndex = tapeLoopDataWritePointer - tapeLoopDataStartPointer;
	int ceiledDelayInSamples = std::ceil(delayInSamples);
	int indexToRetrieve1 = currentIndex - flooredDelayInSamples;
	int indexToRetrieve2 = currentIndex - ceiledDelayInSamples;

	float sample1;
	if (indexToRetrieve1 >= 0) {
		sample1 = tapeLoopData[indexToRetrieve1];
	}
	else {
		sample1 = tapeLoopData[tapeLoopData.size() + indexToRetrieve1];
	}

	float sample2;
	if (indexToRetrieve2 >= 0) {
		sample2 = tapeLoopData[indexToRetrieve2];
	}
	else {
		sample2 = tapeLoopData[tapeLoopData.size() + indexToRetrieve2];
	}

	return linearlyInterpolateBetweenSamples(sample1, sample2, delayInSamples);
}

float TapeLoop::linearlyInterpolateBetweenSamples(float sample1, float sample2, double delayInSamples) {
	double fraction = delayInSamples - (int)delayInSamples;
	double remainder = 1 - fraction;

	return (float)((fraction*sample2) + (remainder*sample1));
}

void TapeLoop::getInterpolatedDelayedBufferFromTape(float* inputPointer, double delayInSamples, int numberOfSamplesToGet, double tapeSpeed) {
	int flooredDelayInSamples = std::floor(delayInSamples);

	// If it's already at a whole number, continue as always using regular non-fractional method
	if (flooredDelayInSamples == delayInSamples) {
		return getDelayedBufferFromTape(inputPointer, int(delayInSamples), numberOfSamplesToGet, tapeSpeed);
	}

	std::vector<float> gotFromTape;
	gotFromTape.resize(numberOfSamplesToGet + 1);
	float* gotFromTapeWritePointer = &gotFromTape[0];

	getDelayedBufferFromTape(gotFromTapeWritePointer, int(delayInSamples + 1), numberOfSamplesToGet + 1, tapeSpeed);

	for (int i = 0; i < numberOfSamplesToGet; ++i) {
		inputPointer[i] = linearlyInterpolateBetweenSamples(gotFromTape[i + 1], gotFromTape[i], delayInSamples);
	}
	return;
}

// Bypasses catmull, used for applying feedback
void TapeLoop::getDelayedBufferFromTape(float* inputPointer, int delayInSamples, int numberOfSamplesToGet) {
	float* readPointerCopy = tapeLoopDataReadPointer;

	// Bring RP up until write Pointer
	tapeLoopDataReadPointer = tapeLoopDataWritePointer;
	int currentIndex = tapeLoopDataReadPointer - tapeLoopDataStartPointer;
	int indexToRetrieve = currentIndex - delayInSamples;
	if (indexToRetrieve >= 0) {
		tapeLoopDataReadPointer = tapeLoopDataReadPointer - delayInSamples;
		return readDirectlyFromTape(inputPointer, numberOfSamplesToGet);
	}
	else {
		tapeLoopDataReadPointer = ((float*)tapeLoopDataEndPointer + 1) + indexToRetrieve;
		return readDirectlyFromTape(inputPointer, numberOfSamplesToGet);
	}
}

void TapeLoop::getDelayedBufferFromTape(float* inputPointer, int delayInSamples, int numberOfSamplesToGet, double tapeSpeed) {
	float* readPointerCopy = tapeLoopDataReadPointer;

	// Bring RP up until write Pointer
	tapeLoopDataReadPointer = tapeLoopDataWritePointer;
	int currentIndex = tapeLoopDataReadPointer - tapeLoopDataStartPointer;
	int indexToRetrieve = currentIndex - delayInSamples;
	if (indexToRetrieve >= 0) {
		tapeLoopDataReadPointer = tapeLoopDataReadPointer - delayInSamples;
		return getFromTape(inputPointer, numberOfSamplesToGet, tapeSpeed);
	}
	else {
		tapeLoopDataReadPointer = ((float*)tapeLoopDataEndPointer + 1) + indexToRetrieve;
		return getFromTape(inputPointer, numberOfSamplesToGet, tapeSpeed);
	}
}

void TapeLoop::getFromTape(float* outputPointer, int numOfSamplesToGet) {
	getFromTape(outputPointer, numOfSamplesToGet, 1.0);
}

void TapeLoop::getFromTape(float* outputPointer, int numOfSamplesToGet, double speed) {
	int actualNumberOfSamplesRequiredFromTape = 0;
	actualNumberOfSamplesRequiredFromTape = std::round((double)numOfSamplesToGet * speed);

	// Go and re-set the actual, accurate speed we want to use, now we know exactly how many samples we're retreiving
	double actualSpeed = actualNumberOfSamplesRequiredFromTape / (double)numOfSamplesToGet;
	float *results = &resultsVector[0];
	readDirectlyFromTape(results, actualNumberOfSamplesRequiredFromTape);
	float *catmullResultsPointer = &catmullResults[0];
	readCatmull.process(actualSpeed, results, catmullResultsPointer, numOfSamplesToGet, actualNumberOfSamplesRequiredFromTape, false);
	if (currentlyFiltering) {
		filter.processSamples(catmullResultsPointer, numOfSamplesToGet);
	}

	std::copy(catmullResultsPointer, catmullResultsPointer + numOfSamplesToGet, outputPointer);
}

// Returns tapeLoopData by val. Shouldn't use other than for testing/validation purposes
std::vector<float> TapeLoop::getTapeLoopData() {
	return tapeLoopData;
}

// Returns tapeLoopData by val. Shouldn't use other than for testing/validation purposes
void TapeLoop::resetTapeLoopDataReadPointer() {
	tapeLoopDataReadPointer = &tapeLoopData[0];
}


// Private method for TapeLoop to use when requiring actual raw data from tape (doesn't pass through catmull)
void TapeLoop::readDirectlyFromTape(float* outputPointer, int numOfSamplesToGet) {

	// get some stuff from tapeLoopData
	if (tapeLoopDataReadPointer + numOfSamplesToGet <= tapeLoopDataEndPointer) {
		// If theres enough space on our tape, whack it in.
		std::copy(tapeLoopDataReadPointer, tapeLoopDataReadPointer + numOfSamplesToGet, outputPointer);
		// Move the write pointer so that we know where to start
		tapeLoopDataReadPointer = tapeLoopDataReadPointer + numOfSamplesToGet;
	}

	// This triggers only when it gets right to the end apparently, and has to fill it right up
	else if (tapeLoopDataReadPointer + (numOfSamplesToGet - 1) <= tapeLoopDataEndPointer) {

		// If theres enough space on our tape, whack it in.
		std::copy(tapeLoopDataReadPointer, tapeLoopDataReadPointer + numOfSamplesToGet, outputPointer);

		// Move the write pointer so that we know where to start
		tapeLoopDataReadPointer = &tapeLoopData[0];
	}
	else {
		std::copy(tapeLoopDataReadPointer, &tapeLoopData[tapeLoopData.size() - 1], outputPointer);
		int numberOfValuesCopiedBeforeWrap = tapeLoopDataEndPointer - tapeLoopDataReadPointer;
		outputPointer = outputPointer + numberOfValuesCopiedBeforeWrap;

		// Wrap
		tapeLoopDataReadPointer = &tapeLoopData[0];

		std::copy(tapeLoopDataReadPointer, tapeLoopDataReadPointer + (numOfSamplesToGet - numberOfValuesCopiedBeforeWrap), outputPointer);

		tapeLoopDataReadPointer = tapeLoopDataReadPointer + (numOfSamplesToGet - numberOfValuesCopiedBeforeWrap);

		// Find where we're currently at in array & wrap the pointer (seems like a rounabout way of doing things!)
		// I called it an imaginary index because it's too far: it's left the vector due to the wrapping required
		int currentImaginaryIndex = tapeLoopDataReadPointer - tapeLoopDataStartPointer;
		tapeLoopDataReadPointer = &tapeLoopData[currentImaginaryIndex % tapeLoopData.size()];
	}
}

// Private method for TapeLoop to use when recording actual raw data from tape (doesn't pass through catmull)
void TapeLoop::recordDirectlyToTape(const float* inputPointer, int numOfSamplesToRecord) {
	if (tapeLoopDataWritePointer + numOfSamplesToRecord <= tapeLoopDataEndPointer) {

		// If theres enough space on our tape, whack it in.
		std::copy(inputPointer, inputPointer + numOfSamplesToRecord, tapeLoopDataWritePointer);

		// Move the write pointer so that we know where to start
		tapeLoopDataWritePointer = tapeLoopDataWritePointer + numOfSamplesToRecord;
	}

	// This triggers only when it gets right to the end apparently, and has to fill it right up
	else if (tapeLoopDataWritePointer + (numOfSamplesToRecord - 1) <= tapeLoopDataEndPointer) {

		// If theres enough space on our tape, whack it in.
		std::copy(inputPointer, inputPointer + numOfSamplesToRecord, tapeLoopDataWritePointer);

		// Move the write pointer so that we know where to start
		tapeLoopDataWritePointer = &tapeLoopData[0];
	}
	else {

		// Fill it up until the end since tapeLoopDataEndPointer points to the last element in the array, we have to add 1 to it to find the actual stop point
		const float *partialInputReadPointer = inputPointer + ((tapeLoopDataEndPointer + 1) - tapeLoopDataWritePointer);
		std::copy(inputPointer, partialInputReadPointer, tapeLoopDataWritePointer);

		// Wrap
		tapeLoopDataWritePointer = &tapeLoopData[0];

		std::copy(partialInputReadPointer, inputPointer + numOfSamplesToRecord, tapeLoopDataWritePointer);
		int numOfWrappedSamples = (inputPointer + numOfSamplesToRecord) - partialInputReadPointer;
		tapeLoopDataWritePointer = tapeLoopDataWritePointer + numOfWrappedSamples;

		// Find where we're currently at in array & wrap the pointer (seems like a rounabout way of doing things!)
		// I called it an imaginary index because it's too far: it's left the vector due to the wrapping required
		int currentImaginaryIndex = tapeLoopDataWritePointer - tapeLoopDataStartPointer;
		tapeLoopDataWritePointer = &tapeLoopData[currentImaginaryIndex % tapeLoopData.size()];
	}
}


