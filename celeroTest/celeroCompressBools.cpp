
#pragma warning(push)
#pragma warning(disable:4251) // warning C4251: celero::Result::pimpl': class 'celero::Pimpl<celero::Result::Impl>' needs to have dll-interface to be used by clients of class 'celero::Result'
#include "celero\Celero.h"
#pragma warning(pop)

#include <bitset>
#include <iostream>
#include <random>
#include <omp.h>

CELERO_MAIN;

static const int SamplesCount = 5;
static const int IterationsCount = 0;
static const int ThresholdValue = 254;

void Checker(bool val, bool reference, uint64_t i)
{
	if (val != reference)
		std::cout << "Mismatch at index: " << i << "\n";
}

class CompressBoolsFixture : public celero::TestFixture
{
public:
	virtual std::vector<std::pair<int64_t, uint64_t>> getExperimentValues() const override
	{
		std::vector<std::pair<int64_t, uint64_t>> problemSpace;

		const int64_t stepCount = MAX_ARRAY_LEN / BENCH_STEPS;

		for (int64_t i = 0; i < BENCH_STEPS; i++)
		{
			// ExperimentValues is part of the base class and allows us to specify
			// some values to control various test runs to end up building a nice graph.
			problemSpace.push_back(std::make_pair(stepCount + i * stepCount, uint64_t(i+1)));
		}

		return problemSpace;
	}

	/// Before each run, build a vector of random integers.
	virtual void setUp(int64_t experimentValue) override
	{
		arrayLength = static_cast<size_t>(experimentValue);
		inputValues.reset(new int[arrayLength]);
		referenceValues.reset(new bool[arrayLength]);
		

		std::mt19937 gen(0); //Standard mersenne_twister_engine seeded with 0, constant
		std::uniform_int_distribution<> dist(0, 255);

		// set every byte, copute reference values
		for (size_t i = 0; i < experimentValue; ++i)
		{
			inputValues[i] = dist(gen);
			referenceValues[i] = inputValues[i] > ThresholdValue;
		}
	}

#ifdef _DEBUG
	static const int64_t MAX_ARRAY_LEN{ 5000 };
	static const int64_t BENCH_STEPS{ 1 };
#else
	static const int64_t MAX_ARRAY_LEN{ 500000 };
	static const int64_t BENCH_STEPS{ 5 };
#endif
	
	size_t arrayLength{ 0 };
	std::unique_ptr<int[]> inputValues;
	std::unique_ptr<bool[]> referenceValues;
};

class NoPackingFixture : public CompressBoolsFixture
{
public:
	virtual void setUp(int64_t experimentValue) override
	{
		CompressBoolsFixture::setUp(experimentValue);

		outputValues.reset(new bool[static_cast<unsigned int>(arrayLength)]);
	}

	virtual void tearDown()
	{
		for (size_t i = 0; i < arrayLength; ++i)
			Checker(outputValues[i], referenceValues[i], i);
	}

	std::unique_ptr<bool[]> outputValues;
};

BASELINE_F(CompressBoolsTest, NoPackingVersion, NoPackingFixture, SamplesCount, IterationsCount)
{
	for (size_t i = 0; i < arrayLength; ++i)
	{
		outputValues[i] = inputValues[i] > ThresholdValue;
	}
}

class StdBitsetFixture : public CompressBoolsFixture
{
public:
	virtual void tearDown()
	{
		for (size_t i = 0; i < arrayLength; ++i)
			Checker(outputBitset[i], referenceValues[i], i);
	}

	std::bitset<MAX_ARRAY_LEN> outputBitset;
};

// Run an automatic baseline.  
// Celero will help make sure enough samples are taken to get a reasonable measurement
BENCHMARK_F(CompressBoolsTest, StdBitset, StdBitsetFixture, SamplesCount, IterationsCount)
{
	for (size_t i = 0; i < arrayLength; ++i)
		outputBitset.set(i, inputValues[i] > ThresholdValue);
}

class StdVectorFixture : public CompressBoolsFixture
{
public:
	virtual void setUp(int64_t experimentValue) override
	{
		CompressBoolsFixture::setUp(experimentValue);

		outputVector.resize(static_cast<unsigned int>(experimentValue));
	}

	virtual void tearDown()
	{
		for (size_t i = 0; i < arrayLength; ++i)
			Checker(outputVector[i], referenceValues[i], i);
	}

	std::vector<bool> outputVector;
};

BENCHMARK_F(CompressBoolsTest, StdVector, StdVectorFixture, SamplesCount, IterationsCount)
{
	for (size_t i = 0; i < arrayLength; ++i)
		outputVector[i] = inputValues[i] > ThresholdValue;
}

class ManualVersionFixture : public CompressBoolsFixture
{
public:
	virtual void setUp(int64_t experimentValue) override
	{
		CompressBoolsFixture::setUp(experimentValue);

		numBytes = static_cast<unsigned int>((experimentValue + 7) / 8);
		numFullBytes = static_cast<unsigned int>((experimentValue) / 8);
		outputValues.reset(new uint8_t[numBytes]);
	}

	virtual void tearDown()
	{
		for (size_t i = 0; i < arrayLength; ++i)
			Checker(getBoolAt(i), referenceValues[i], i);
	}

	bool getBoolAt(int64_t pos)
	{
		auto bytePos = pos / 8;
		auto bitPos = pos % 8;
		return (*(outputValues.get() + bytePos)) & (1 << bitPos);
	}

	unsigned int numBytes;
	unsigned int numFullBytes;
	std::unique_ptr<uint8_t[]> outputValues;
};

BENCHMARK_F(CompressBoolsTest, SingleVarVersion, ManualVersionFixture, SamplesCount, IterationsCount)
{
	uint8_t OutByte = 0;
	int shiftCounter = 0;

	auto pInputData = inputValues.get();
	auto pOutputByte = outputValues.get();

	for (int64_t i = 0; i < arrayLength; ++i)
	{
		if (*pInputData > ThresholdValue)
			OutByte |= (1 << shiftCounter);

		pInputData++;
		shiftCounter++;

		if (shiftCounter > 7)
		{
			*pOutputByte++ = OutByte;
			OutByte = 0;
			shiftCounter = 0;
		}
	}

	// our byte might be incomplete, so we need to handle this:
	if (arrayLength & 7)
	{
		*pOutputByte++ = OutByte;
	}
}

BENCHMARK_F(CompressBoolsTest, NotDepentendVersion, ManualVersionFixture, SamplesCount, IterationsCount)
{
	uint8_t Bits[8] = { 0 };
	const int64_t lenDivBy8 = (arrayLength / 8) * 8;

	auto pInputData = inputValues.get();
	auto pOutputByte = outputValues.get();

	for (int64_t j = 0; j < lenDivBy8; j += 8)
	{
		Bits[0] = pInputData[0] > ThresholdValue ? 0x01 : 0;
		Bits[1] = pInputData[1] > ThresholdValue ? 0x02 : 0;
		Bits[2] = pInputData[2] > ThresholdValue ? 0x04 : 0;
		Bits[3] = pInputData[3] > ThresholdValue ? 0x08 : 0;
		Bits[4] = pInputData[4] > ThresholdValue ? 0x10 : 0;
		Bits[5] = pInputData[5] > ThresholdValue ? 0x20 : 0;
		Bits[6] = pInputData[6] > ThresholdValue ? 0x40 : 0;
		Bits[7] = pInputData[7] > ThresholdValue ? 0x80 : 0;

		*pOutputByte++ = Bits[0] | Bits[1] | Bits[2] | Bits[3] | Bits[4] | Bits[5] | Bits[6] | Bits[7];
		pInputData += 8;
	}
	if (arrayLength & 7)
	{
		auto RestW = arrayLength & 7;
		memset(Bits, 0, 8);
		for (long long i = 0; i < RestW; ++i)
		{
			Bits[i] = *pInputData == ThresholdValue ? 1 << i : 0;
			pInputData++;
		}
		*pOutputByte++ = Bits[0] | Bits[1] | Bits[2] | Bits[3] | Bits[4] | Bits[5] | Bits[6] | Bits[7];
	}
}

struct bool8 {
	uint8_t val0 : 1;
	uint8_t val1 : 1;
	uint8_t val2 : 1;
	uint8_t val3 : 1;
	uint8_t val4 : 1;
	uint8_t val5 : 1;
	uint8_t val6 : 1;
	uint8_t val7 : 1;
};

class PackedStructFixture : public CompressBoolsFixture
{
public:
	virtual void setUp(int64_t experimentValue) override
	{
		CompressBoolsFixture::setUp(experimentValue);

		numBytes = static_cast<unsigned int>((experimentValue + 7) / 8);
		numFullBytes = static_cast<unsigned int>((experimentValue) / 8);
		outputValues.reset(new bool8[numBytes]);
	}

	virtual void tearDown()
	{
		for (size_t i = 0; i < arrayLength; ++i)
			Checker(getBoolAt(i), referenceValues[i], i);
	}

	bool getBoolAt(int64_t pos)
	{
		auto bytePos = pos / 8;
		auto bitPos = pos % 8;
		switch (bitPos)
		{
		case 0:return (*(outputValues.get() + bytePos)).val0;
		case 1:return (*(outputValues.get() + bytePos)).val1;
		case 2:return (*(outputValues.get() + bytePos)).val2;
		case 3:return (*(outputValues.get() + bytePos)).val3;
		case 4:return (*(outputValues.get() + bytePos)).val4;
		case 5:return (*(outputValues.get() + bytePos)).val5;
		case 6:return (*(outputValues.get() + bytePos)).val6;
		}
		return (*(outputValues.get() + bytePos)).val7;
	}

	unsigned int numBytes;
	unsigned int numFullBytes;
	std::unique_ptr<bool8[]> outputValues;
};

BENCHMARK_F(CompressBoolsTest, PackedStructVersion, PackedStructFixture, SamplesCount, IterationsCount)
{
	const int64_t lenDivBy8 = (arrayLength / 8) * 8;

	auto pInputData = inputValues.get();
	auto pOutputByte = outputValues.get();

	bool8 out;

	for (int64_t j = 0; j < lenDivBy8; j += 8)
	{
		out.val0 = pInputData[0] > ThresholdValue;
		out.val1 = pInputData[1] > ThresholdValue;
		out.val2 = pInputData[2] > ThresholdValue;
		out.val3 = pInputData[3] > ThresholdValue;
		out.val4 = pInputData[4] > ThresholdValue;
		out.val5 = pInputData[5] > ThresholdValue;
		out.val6 = pInputData[6] > ThresholdValue;
		out.val7 = pInputData[7] > ThresholdValue;

		*pOutputByte++ = out;
		pInputData += 8;
	}
	if (arrayLength & 7)
	{
		auto RestW = arrayLength & 7;
		out = { 0, 0, 0, 0, 0, 0, 0, 0 };
		if (RestW > 6) out.val6 = pInputData[6] > ThresholdValue;
		if (RestW > 5) out.val5 = pInputData[5] > ThresholdValue;
		if (RestW > 4) out.val4 = pInputData[4] > ThresholdValue;
		if (RestW > 3) out.val3 = pInputData[3] > ThresholdValue;
		if (RestW > 2) out.val2 = pInputData[2] > ThresholdValue;
		if (RestW > 1) out.val1 = pInputData[1] > ThresholdValue;
		if (RestW > 0) out.val0 = pInputData[0] > ThresholdValue;
		*pOutputByte++ = out;
	}
}

BENCHMARK_F(CompressBoolsTest, WithOpenMP, ManualVersionFixture, SamplesCount, IterationsCount)
{
	uint8_t Bits[8];
#pragma omp parallel for private (Bits)
	for (int i = 0; i < numFullBytes; ++i)
	{
		auto pInputData = inputValues.get() + i*8;
		Bits[0] = pInputData[0] > ThresholdValue ? 0x01 : 0;
		Bits[1] = pInputData[1] > ThresholdValue ? 0x02 : 0;
		Bits[2] = pInputData[2] > ThresholdValue ? 0x04 : 0;
		Bits[3] = pInputData[3] > ThresholdValue ? 0x08 : 0;
		Bits[4] = pInputData[4] > ThresholdValue ? 0x10 : 0;
		Bits[5] = pInputData[5] > ThresholdValue ? 0x20 : 0;
		Bits[6] = pInputData[6] > ThresholdValue ? 0x40 : 0;
		Bits[7] = pInputData[7] > ThresholdValue ? 0x80 : 0;

		outputValues.get()[i] = Bits[0] | Bits[1] | Bits[2] | Bits[3] | Bits[4] | Bits[5] | Bits[6] | Bits[7];
	}
	if (numFullBytes < numBytes)
	{
		uint8_t Bits[8] = { 0 };
		auto RestW = arrayLength & 7;
		auto pInputData = inputValues.get() + numFullBytes * 8;
		for (long long i = 0; i < RestW; ++i)
		{
			Bits[i] = *pInputData == ThresholdValue ? 1 << i : 0;
			pInputData++;
		}
		outputValues.get()[numFullBytes] = Bits[0] | Bits[1] | Bits[2] | Bits[3] | Bits[4] | Bits[5] | Bits[6]/* | Bits[7]*/;
	}
}