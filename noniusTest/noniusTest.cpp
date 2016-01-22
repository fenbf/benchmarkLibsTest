#define NONIUS_RUNNER
//#define NONIUS_USE_BOOST_CHRONO
#include "nonius-1.1.2\nonius\nonius.h++"
#include "../commonTest.h"


NONIUS_BENCHMARK("IntToStringTest1000", [] {
	return IntToStringConversionTest(TEST_NUM_COUNT1000);
})

NONIUS_BENCHMARK("DoubleToStringTest1000", [] {
	return DoubleToStringConversionTest(TEST_NUM_COUNT1000);
})
