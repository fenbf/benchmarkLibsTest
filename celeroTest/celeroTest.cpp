#include "celero\Celero.h"
#include "../commonTest.h"

CELERO_MAIN;

// Run an automatic baseline.  
// Celero will help make sure enough samples are taken to get a reasonable measurement
BASELINE(IntToStringTest, Baseline10, 0, 100)
{
	celero::DoNotOptimizeAway(IntToStringConversionTest(TEST_NUM_COUNT10));
}

BENCHMARK(IntToStringTest, Baseline1000, 0, 100)
{
	celero::DoNotOptimizeAway(IntToStringConversionTest(TEST_NUM_COUNT1000));
}

BASELINE(DoubleToStringTest, Baseline10, 10, 100)
{
	celero::DoNotOptimizeAway(DoubleToStringConversionTest(TEST_NUM_COUNT10));
}

BENCHMARK(DoubleToStringTest, Baseline1000, 10, 100)
{
	celero::DoNotOptimizeAway(DoubleToStringConversionTest(TEST_NUM_COUNT1000));
}
