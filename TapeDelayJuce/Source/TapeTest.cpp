#include "PluginProcessor.h"
#include "TapeTest.h"

float * TapeTest::getFakeAudioBuffer(int length)
{
	// resize to length
	fakeAudioBuffer.resize(length);
	// fill with values counting up 0.0001, 0.0002 etc
	for (int i = 0; i < fakeAudioBuffer.size(); ++i) {
		float num = (float)i / (float)1000;
		fakeAudioBuffer[i] = num;
	}
	return &fakeAudioBuffer[0];
}

float * TapeTest::getRealisticFakeAudioBuffer(int bufferLength)
{
	fakeAudioBuffer.resize(bufferLength);

	// fill with values counting up 1, 2 etc.
	for (int i = 0; i < fakeAudioBuffer.size(); ++i) {
		float num = (float)realisticCounter;
		fakeAudioBuffer[i] = num;
		realisticCounter++;
	}
	realisticTimesCalled++;
	return &fakeAudioBuffer[0];
}

void TapeTest::runTest()
{
	bufferLength = 512;
	testTape.setTapeLength(1, 48000.000000000000);

	beginTest("Test record to tape");

	expect(test_recordToTape());

	beginTest("Test get from tape");

	expect(test_getFromTape());

	beginTest("Test get from tape (fast)");

	expect(test_getFromTape_fast());

	beginTest("Test get delayed sample from tape");

	expect(test_getDelayedSampleFromTape());

	beginTest("Test delay unit");

	expect(test_DelayUnitProcess());
}

bool TapeTest::test_recordToTape() {
	int numberOfPasses = 1000;
	for (int i = 0; i < numberOfPasses; ++i) {
		testTape.recordToTape(getRealisticFakeAudioBuffer(bufferLength), bufferLength);
	}
	std::vector<float> testTapeLoopData = testTape.getTapeLoopData();

	// Goes to false if not at least one value has been written to 0 after wrapping
	// Will not go to false if buffer just completely filled
	// In the case that the delay buffer wasn't fully utilised
	if (numberOfPasses * bufferLength <= testTapeLoopData.size()) {
		wrapped = false;
	}
	float currentSampleVal = 0.0f;
	float nextSampleVal = 0.0f;
	float expectedVal;

	// Will be set to point at the first element of the sequence that *hasn't* been overwritten after wrap, causing a drop off in the signal (expected)
	bool dropOffPointDetected = false;

	// Validate up until the end
	for (int i = 0; i < testTapeLoopData.size(); ++i) {
		// Validate this and the one after, until the end when you would run out
		currentSampleVal = testTapeLoopData[i];
		expectedVal = currentSampleVal + float(1);
		if (i != testTapeLoopData.size() - 1) {
			nextSampleVal = testTapeLoopData[i+1];
			if (nextSampleVal != expectedVal) {
				// Detect whether this is just the signal drop off point, or an actual error
				// Could just leave dropOffPointDetected only, might not need second conditions
				if (!dropOffPointDetected && ((int)nextSampleVal % bufferLength == 0 || (int)nextSampleVal % 32 == 0)) {
					dropOffPointDetected = true;
					//dropOffPoint = i+1;
				}
				else if (!wrapped && (int)nextSampleVal == 0) {
					// do nothing
				}
				else {
					return false;
				}
			}
		}
		// Break when nextSampleVal would be off the end of the array
		else {
			break;
		}
	}

	// Check wrap around working okay
	if (testTapeLoopData[0] != expectedVal && wrapped) {
		return false;
	}

	return true;
}

bool TapeTest::test_getFromTape()
{
	// Take a copy of testTape, then at the end we can check that is hasn't been disrupted!
	const std::vector<float> originalTapeLoopData = testTape.getTapeLoopData();
	std::vector<float> testTapeLoopData = testTape.getTapeLoopData();

	// This is emulating the output audio buffer, just 1 channel
	AudioBuffer<float> testBuffer(1, bufferLength);
	auto* testChannelData = testBuffer.getWritePointer(0);
	const auto* readTestChannelData = testBuffer.getReadPointer(0);

	// The number of repetitions could be randomised - this is emulating processBlock
	for (int i = 0; i <= 110; ++i) {
		testTape.getFromTape(testChannelData, bufferLength);

		float currentSampleVal = 0.0f;
		float nextSampleVal = 0.0f;
		float expectedVal;

		// Will be set to point at the first element of the sequence that *hasn't* been overwritten after wrap, causing a drop off in the signal (expected)
		bool dropOffPointDetected = false;

		// Check for completely empty/broken test
		if (readTestChannelData[0] == -431602080.00) {
			return false;
		}

		// Validate up until the end
		for (int i = 0; i < testBuffer.getNumSamples(); ++i) {

			// Validate this and the one after, until the end when you would run out
			currentSampleVal = readTestChannelData[i];
			expectedVal = currentSampleVal + float(1);
			if (i != testBuffer.getNumSamples() - 1) {
				nextSampleVal = readTestChannelData[i + 1];
				if (nextSampleVal != expectedVal) {

					// Detect whether this is just the signal drop off point, or an actual error
					// If you get more problems with this perhaps remove second condition
					if (!dropOffPointDetected && ((int)nextSampleVal % bufferLength == 0 || (int)nextSampleVal % 32 == 0)) {
						dropOffPointDetected = true;
					}
					else if (!wrapped && (int)nextSampleVal == 0) {
						// do nothing
					}
					else {
						return false;
					}
				}
			}

			// Break when nextSampleVal would be off the end of the array
			else {
				break;
			}
		}
	}
	return true;
}


bool TapeTest::test_getFromTape_fast()
{
	// Take a copy of testTape, then at the end we can check that is hasn't been disrupted!
	const std::vector<float> originalTapeLoopData = testTape.getTapeLoopData();
	std::vector<float> testTapeLoopData = testTape.getTapeLoopData();

	// This is emulating the output audio buffer, just 1 channel
	AudioBuffer<float> testBuffer(1, bufferLength);
	auto* testChannelData = testBuffer.getWritePointer(0);
	const auto* readTestChannelData = testBuffer.getReadPointer(0);
	std::vector<float> testChannelDataFloatArray;
	double speed = 1;

	// The number of repetitions could be randomised - this is emulating processBlock
	for (int i = 0; i <= 110; ++i) {
		float currentSampleVal = 0.0f;
		float nextSampleVal = 0.0f;
		float expectedVal;

		// Will be set to point at the first element of the sequence that *hasn't* been overwritten after wrap, causing a drop off in the signal (expected)
		bool dropOffPointDetected = false;

		//Just bring up a vector so we can inspect it in the debugger
		testChannelDataFloatArray.resize(bufferLength);
		float* testChannelDataFloatArrayWritePointer = &testChannelDataFloatArray[0];
		std::copy(readTestChannelData, readTestChannelData + bufferLength, testChannelDataFloatArrayWritePointer);
		testTape.getFromTape(testChannelData, bufferLength);

		// Validate up until the end
		for (int j = 0; j < testBuffer.getNumSamples(); ++j) {
			// Validate this and the one after, until the end when you would run out
			currentSampleVal = readTestChannelData[j];
			expectedVal = currentSampleVal + float(speed);
			if (j != testBuffer.getNumSamples() - 1) {
				nextSampleVal = readTestChannelData[j + 1];

				if (nextSampleVal != expectedVal) {
					// Detect whether this is just the signal drop off point, or an actual error
					// If you get more problems with this perhaps remove second condition
					if (!dropOffPointDetected) {
						dropOffPointDetected = true;
					}
					else if (!wrapped && (int)nextSampleVal == 0) {
						// do nothing
					}
					else {
						return false;
					}
				}
			}
			// Break when nextSampleVal would be off the end of the array
			else {
				break;
			}
		}
	}
	return true;
}


bool TapeTest::test_getDelayedSampleFromTape() {
	float test = testTape.getDelayedSampleFromTape(32000);
	return true;
}


bool TapeTest::test_DelayUnitProcess() {
	DelayUnit testDelayUnit;
	testDelayUnit.prepareDelayUnit(15, 48000.000000000000, bufferLength);
	testDelayUnit.enableDebug(true);
	realisticTimesCalled = 0;
	realisticCounter = 0;
	std::vector<float> inspectionTestVector;
	inspectionTestVector.resize(bufferLength);
	float* inspectionTestVectorWritePointer = &inspectionTestVector[0];
	float* realisticFakeAudioBuffer = getRealisticFakeAudioBuffer(bufferLength);
	std::copy(realisticFakeAudioBuffer, realisticFakeAudioBuffer + bufferLength, inspectionTestVectorWritePointer);
	testDelayUnit.process(realisticFakeAudioBuffer, bufferLength);
	for (int i = 0; i <= 55; ++i) {
		testDelayUnit.process(getRealisticFakeAudioBuffer(bufferLength), bufferLength);
	}
	testDelayUnit.changeDelayTimeBbd(537);
	for (int i = 0; i <= 55; ++i) {
		testDelayUnit.process(getRealisticFakeAudioBuffer(bufferLength), bufferLength);
	}
	return true;
}