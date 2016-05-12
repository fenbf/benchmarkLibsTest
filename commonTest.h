#pragma once

#include <vector>
#include <numeric>
#include <string>

#define TEST_NUM_COUNT10 10
#define TEST_NUM_COUNT100 100
#define TEST_NUM_COUNT1000 1000

auto IntToStringConversionTest(int count)
{
	std::vector<int> inputNumbers(count);
	std::vector<std::string> outNumbers;

	std::iota(std::begin(inputNumbers), std::end(inputNumbers), 0);
	for (auto &num : inputNumbers)
		outNumbers.push_back(std::to_string(num));

	return outNumbers;
}

auto DoubleToStringConversionTest(int count)
{
	std::vector<double> inputNumbers(count);
	std::vector<std::string> outNumbers;

	std::iota(std::begin(inputNumbers), std::end(inputNumbers), 0.12345);
	for (auto &num : inputNumbers)
		outNumbers.push_back(std::to_string(num));

	return outNumbers;
}