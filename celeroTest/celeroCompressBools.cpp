
#pragma warning(push)
#pragma warning(disable:4251) // warning C4251: celero::Result::pimpl': class 'celero::Pimpl<celero::Result::Impl>' needs to have dll-interface to be used by clients of class 'celero::Result'
#include "celero\Celero.h"
#pragma warning(pop)

#include <bitset>
#include <iostream>
#include <random>
#include <omp.h>
#include <emmintrin.h>

CELERO_MAIN;

static const int SamplesCount = 5;
static const int IterationsCount = 0; // Celero will pick the number...
static const int ThresholdValue = 127;
static const int MAX_DIST = 255;

void Checker(bool val, bool reference, uint64_t i)
{
	if (val != reference)
		std::cout << "Mismatch at index: " << i << "\n";
}

bool GetBoolAt(int64_t pos, uint8_t* outputValues)
{
	auto bytePos = pos / 8;
	auto bitPos = pos % 8;
	return (*(outputValues + bytePos)) & (1 << bitPos);
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
		std::uniform_int_distribution<> dist(0, MAX_DIST);

		// set every byte, copute reference values
		for (size_t i = 0; i < experimentValue; ++i)
		{
			inputValues[i] = dist(gen);
			referenceValues[i] = inputValues[i] > ThresholdValue;
		}
	}

#ifdef _DEBUG
	static const int64_t MAX_ARRAY_LEN{ 100 };
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
		outputValues[i] = inputValues[i] > ThresholdValue;
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
			Checker(GetBoolAt(i, outputValues.get()), referenceValues[i], i);
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
		// note that we'll use max 7 elements, so max is Bits[6]... (otherwise we would get another full byte)
		auto RestW = arrayLength & 7;
		memset(Bits, 0, 8);
		for (long long i = 0; i < RestW; ++i)
		{
			Bits[i] = *pInputData > ThresholdValue ? 1 << i : 0;
			pInputData++;
		}
		*pOutputByte++ = Bits[0] | Bits[1] | Bits[2] | Bits[3] | Bits[4] | Bits[5] | Bits[6];
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
			Bits[i] = *pInputData > ThresholdValue ? 1 << i : 0;
			pInputData++;
		}
		outputValues.get()[numFullBytes] = Bits[0] | Bits[1] | Bits[2] | Bits[3] | Bits[4] | Bits[5] | Bits[6];
	}
}

class SimdVersionFixture : public CompressBoolsFixture
{
public:
	virtual void setUp(int64_t experimentValue) override
	{
		CompressBoolsFixture::setUp(experimentValue);

		numBytes = static_cast<unsigned int>((experimentValue + 7) / 8);
		numFullBytes = static_cast<unsigned int>((experimentValue) / 8);
		alignedOutputValues = (uint8_t *)_aligned_malloc(numBytes, 16);

		signedInputValues.reset(new int8_t[arrayLength]);
		for (size_t i = 0; i < arrayLength; ++i)
			signedInputValues[i] = inputValues[i] - 128;
	}

	virtual void tearDown()
	{
		for (size_t i = 0; i < arrayLength; ++i)
			Checker(GetBoolAt(i, alignedOutputValues), referenceValues[i], i);

		if (alignedOutputValues)
		{
			_aligned_free(alignedOutputValues);
			alignedOutputValues = nullptr;
		}
	}

	unsigned int numBytes;
	unsigned int numFullBytes;
	uint8_t* alignedOutputValues{ nullptr };
	std::unique_ptr<int8_t[]> signedInputValues;
};

BENCHMARK_F(CompressBoolsTest, SimdVersion, SimdVersionFixture, SamplesCount, IterationsCount)
{
	uint16_t Bits[16] = { 0 };
	const size_t lenDiv16y16 = (arrayLength / 16) * 16; // full packs of 16 values...

	const __m128i sse127 = _mm_set1_epi8(127);
	const int8_t ConvertedThreshold = ThresholdValue - 128;
	const __m128i sseThresholds = _mm_set1_epi8(ConvertedThreshold);

	auto pInputData = signedInputValues.get();
	auto pOutputByte = alignedOutputValues;
	for (size_t j = 0; j < lenDiv16y16; j += 16)
	{
		/*const auto inPixelReg = _mm_set_epi8(pInputData[0], pInputData[1], pInputData[2], pInputData[3],
			pInputData[4], pInputData[5], pInputData[6], pInputData[7],
			pInputData[8], pInputData[9], pInputData[10], pInputData[11],
			pInputData[12], pInputData[13], pInputData[14], pInputData[15]);*/
		const auto in16Values = _mm_set_epi8(pInputData[15], pInputData[14], pInputData[13], pInputData[12],
			pInputData[11], pInputData[10], pInputData[9], pInputData[8],
			pInputData[7], pInputData[6], pInputData[5], pInputData[4],
			pInputData[3], pInputData[2], pInputData[1], pInputData[0]);
		const auto cmpRes = _mm_cmpgt_epi8(in16Values, sseThresholds);
		const auto packed = _mm_movemask_epi8(cmpRes);
		*((uint16_t *)pOutputByte) = static_cast<uint16_t>(packed);
		//*pOutputByte++ = (packed & 0x0000FF00) >> 8;
		//*pOutputByte++ = packed & 0x000000FF;
		pOutputByte += 2;
		pInputData += 16;
	}
	if (arrayLength & 15)
	{
		auto RestW = arrayLength & 15;
		memset(Bits, 0, 16 * sizeof(uint16_t));
		for (size_t i = 0; i < RestW; ++i)
		{
			Bits[i] = *pInputData > ConvertedThreshold ? 1 << i : 0;
			pInputData++;
		}
		*pOutputByte++ = Bits[0] | Bits[1] | Bits[2] | Bits[3] | Bits[4] | Bits[5] | Bits[6] | Bits[7];
		if (RestW > 8)
			*pOutputByte++ = (Bits[8] | Bits[9] | Bits[10] | Bits[11] | Bits[12] | Bits[13] | Bits[14] | Bits[15]) >> 8;
	}
}