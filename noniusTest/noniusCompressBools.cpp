#define NONIUS_RUNNER
//#define NONIUS_USE_BOOST_CHRONO
#include "nonius-1.1.2\nonius\nonius.h++"
#include <memory>
#include <random>
#include <iostream>
#include <omp.h>


const size_t experimentValue = 100000;
static const int ThresholdValueLarge = 254;
static const int ThresholdValueHalf = 127;

std::unique_ptr<int[]> PrepareInputValues()
{
	std::unique_ptr<int[]> inputValues;

	inputValues.reset(new int[experimentValue]);

std::mt19937 gen(0); //Standard mersenne_twister_engine seeded with 0, constant
std::uniform_int_distribution<> dist(0, 255);

// set every byte, copute reference values
for (size_t i = 0; i < experimentValue; ++i)
	inputValues[i] = dist(gen);

	return inputValues;
}

template <int Threshold>
void ComputeNotDependent(int* pInputData, uint8_t *pOutputByte)
{
uint8_t Bits[8] = { 0 };
const int64_t lenDivBy8 = (experimentValue / 8) * 8;

for (int64_t j = 0; j < lenDivBy8; j += 8)
{
	Bits[0] = pInputData[0] > Threshold ? 0x01 : 0;
	Bits[1] = pInputData[1] > Threshold ? 0x02 : 0;
	Bits[2] = pInputData[2] > Threshold ? 0x04 : 0;
	Bits[3] = pInputData[3] > Threshold ? 0x08 : 0;
	Bits[4] = pInputData[4] > Threshold ? 0x10 : 0;
	Bits[5] = pInputData[5] > Threshold ? 0x20 : 0;
	Bits[6] = pInputData[6] > Threshold ? 0x40 : 0;
	Bits[7] = pInputData[7] > Threshold ? 0x80 : 0;

	*pOutputByte++ = Bits[0] | Bits[1] | Bits[2] | Bits[3] | Bits[4] | Bits[5] | Bits[6] | Bits[7];
	pInputData += 8;
}
}

template <int Threshold>
void ComputeSingleVar(int* pInputData, uint8_t *pOutputByte)
{
uint8_t OutByte = 0;
int shiftCounter = 0;

for (int i = 0; i < experimentValue; ++i)
{
	if (*pInputData > Threshold)
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
}

NONIUS_BENCHMARK("NotDependent254", [](nonius::chronometer meter) {
	std::unique_ptr<int[]> inputValues = PrepareInputValues();

	unsigned int numBytes;
	std::unique_ptr<uint8_t[]> outputValues;

	numBytes = static_cast<unsigned int>((experimentValue + 7) / 8);
	outputValues.reset(new uint8_t[numBytes]);

	meter.measure([&inputValues, &outputValues] {
		ComputeNotDependent<ThresholdValueLarge>(inputValues.get(), outputValues.get());
		// the last byte is skipped for simplification
	});
});

NONIUS_BENCHMARK("NotDependent127", [](nonius::chronometer meter) {
	std::unique_ptr<int[]> inputValues = PrepareInputValues();

	unsigned int numBytes;
	std::unique_ptr<uint8_t[]> outputValues;

	numBytes = static_cast<unsigned int>((experimentValue + 7) / 8);
	outputValues.reset(new uint8_t[numBytes]);

	meter.measure([&inputValues, &outputValues] {
		ComputeNotDependent<ThresholdValueHalf>(inputValues.get(), outputValues.get());
	});
});

NONIUS_BENCHMARK("SingleVar254", [](nonius::chronometer meter) {
	std::unique_ptr<int[]> inputValues = PrepareInputValues();

	unsigned int numBytes;
	std::unique_ptr<uint8_t[]> outputValues;

	numBytes = static_cast<unsigned int>((experimentValue + 7) / 8);
	outputValues.reset(new uint8_t[numBytes]);

	meter.measure([&inputValues, &outputValues] {
		ComputeSingleVar<ThresholdValueLarge>(inputValues.get(), outputValues.get());
		// the last byte is skipped for simplification
	});
});

NONIUS_BENCHMARK("SingleVar127", [](nonius::chronometer meter) {
	std::unique_ptr<int[]> inputValues = PrepareInputValues();

	unsigned int numBytes;
	std::unique_ptr<uint8_t[]> outputValues;

	numBytes = static_cast<unsigned int>((experimentValue + 7) / 8);
	outputValues.reset(new uint8_t[numBytes]);

	meter.measure([&inputValues, &outputValues] {
		ComputeSingleVar<ThresholdValueHalf>(inputValues.get(), outputValues.get());
		// the last byte is skipped for simplification
	});
});

NONIUS_BENCHMARK("vectorBool254", [](nonius::chronometer meter) {
	std::unique_ptr<int[]> inputValues = PrepareInputValues();
	std::vector<bool> outputValues;

	outputValues.resize(experimentValue);

	meter.measure([&inputValues, &outputValues] 
	{
		for (size_t i = 0; i < experimentValue; ++i)
			outputValues[i] = inputValues[i] > ThresholdValueLarge;
	});
});

NONIUS_BENCHMARK("vectorBool127", [](nonius::chronometer meter) {
	std::unique_ptr<int[]> inputValues = PrepareInputValues();
	std::vector<bool> outputValues;

	outputValues.resize(experimentValue);

	meter.measure([&inputValues, &outputValues]
	{
		for (size_t i = 0; i < experimentValue; ++i)
			outputValues[i] = inputValues[i] > ThresholdValueHalf;
	});
});

NONIUS_BENCHMARK("baseline254", [](nonius::chronometer meter) {
	std::unique_ptr<int[]> inputValues = PrepareInputValues();
	std::unique_ptr<uint8_t[]> outputValues(new uint8_t[experimentValue]);

	meter.measure([&inputValues, &outputValues]
	{
		for (size_t i = 0; i < experimentValue; ++i)
			outputValues[i] = inputValues[i] > ThresholdValueLarge;
	});
});

NONIUS_BENCHMARK("baseline127", [](nonius::chronometer meter) {
	std::unique_ptr<int[]> inputValues = PrepareInputValues();
std::unique_ptr<uint8_t[]> outputValues(new uint8_t[experimentValue]);

meter.measure([&inputValues, &outputValues]
{
	for (size_t i = 0; i < experimentValue; ++i)
		outputValues[i] = inputValues[i] > ThresholdValueLarge;
});
});