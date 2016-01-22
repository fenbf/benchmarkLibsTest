#include "hayai-master\src\hayai.hpp"
#include "hayai-master\src\hayai_main.hpp"
#include "../commonTest.h"


	BENCHMARK(ToString, IntConversion1000, 10, 100)
	{
		IntToStringConversionTest(TEST_NUM_COUNT1000);
	}

	BENCHMARK(ToString, DoubleConversion1000, 10, 100)
	{
		DoubleToStringConversionTest(TEST_NUM_COUNT1000);
	}

//int main()
//{
//	hayai::ConsoleOutputter consoleOutputter;
//
//	hayai::Benchmarker::AddOutputter(consoleOutputter);
//	hayai::Benchmarker::RunAllTests();
//	return 0;
//}

int main(int argc, char** argv)
{
	// Set up the main runner.
	::hayai::MainRunner runner;

	// Parse the arguments.
	int result = runner.ParseArgs(argc, argv);
	if (result)
	{
		return result;
	}

	// Execute based on the selected mode.
	return runner.Run();
}