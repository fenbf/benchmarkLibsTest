
#include "benchmark/benchmark_api.h"
#include "../commonTest.h"

void IntToString(benchmark::State& state) {
	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(IntToStringConversionTest(state.range_x()));
	}
	state.SetLabel("Hello World");
	state.SetBytesProcessed(1024*1024);
	state.SetItemsProcessed(1000);
}
BENCHMARK(IntToString)->Arg(TEST_NUM_COUNT1000*10)->Unit(benchmark::kMillisecond);
//BENCHMARK(BasicTest)->ThreadPerCpu();

void DoubleToString(benchmark::State& state) {
	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(DoubleToStringConversionTest(state.range_x()));
	}
	state.SetLabel("Foo Bar");
	state.SetBytesProcessed(1024);
	state.SetItemsProcessed(500);
}
BENCHMARK(DoubleToString)->Arg(TEST_NUM_COUNT1000*10)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN()

