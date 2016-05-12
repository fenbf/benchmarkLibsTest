#include "benchmark/benchmark_api.h"
#include "../particles.h"
#include <vector>
#include <memory>

const size_t NUM_PARTICLES = 10000;

template <class Part>
class ParticlesObjVectorFixture : public ::benchmark::Fixture {
public:
	void SetUp(const ::benchmark::State& st) {
		particles = std::vector<Part>(st.range_x());

		for (auto &p : particles)
			p.generate();
	}

	void TearDown(const ::benchmark::State&) {
		particles.clear();
	}

	std::vector<Part> particles;
};

template<typename T> void DoRandomizeAddresses(std::vector<T> &particles)
{
	const auto len = particles.size();
	for (size_t i = 0; i < len / 2; ++i)
	{
		int a = rand() % len;
		int b = rand() % len;
		if (a != b)
			std::swap(particles[a], particles[b]);
	}
}

template <class Part>
class ParticlesPtrVectorFixture : public ::benchmark::Fixture {
public:
	virtual bool WantRandomizeAddresses() { return true; }

	void SetUp(const ::benchmark::State& st) {
		particles = std::vector<std::shared_ptr<Part>>(st.range_x());

		for (auto &p : particles)
			p = std::make_shared<Part>();

		if (WantRandomizeAddresses())
			DoRandomizeAddresses(particles);

		for (auto &p : particles)
			p->generate();
	}

	void TearDown(const ::benchmark::State&) {
		particles.clear();
	}

	std::vector<std::shared_ptr<Part>> particles;
};

template <class Part>
class ParticlesPtrVectorNoRandFixture : public ParticlesPtrVectorFixture<Part>
{
public:
	virtual bool WantRandomizeAddresses() { return false; }
};

template <class Part>
class ParticlesUniquePtrVectorFixture : public ::benchmark::Fixture {
public:
	virtual bool WantRandomizeAddresses() { return true; }

	void SetUp(const ::benchmark::State& st) {
		particles = std::vector<std::unique_ptr<Part>>(st.range_x());

		for (auto &p : particles)
			p = std::make_unique<Part>();

		if (WantRandomizeAddresses())
			DoRandomizeAddresses(particles);

		for (auto &p : particles)
			p->generate();
	}

	void TearDown(const ::benchmark::State&) {
		particles.clear();
	}

	std::vector<std::unique_ptr<Part>> particles;
};

template <class Part>
class ParticlesUniquePtrVectorFixtureNoRand : public ParticlesUniquePtrVectorFixture<Part>
{
public:
	virtual bool WantRandomizeAddresses() { return false; }
};

static void CustomArguments(benchmark::internal::Benchmark* b) {
	for (int i = 0; i <= 5; ++i)
		b->Arg(1024 + i * 2 * 1024);
}

template <class Part>
static void UpdateParticlesObj(std::vector<Part> &particles)
{
	for (size_t u = 0; u < UPDATES; ++u)
	{
		for (auto &p : particles)
			p.update(DELTA_TIME);
	}
}

template<typename T> void UpdateParticlesPtr(std::vector<T> &particles)
{
	for (size_t u = 0; u < UPDATES; ++u)
	{
		for (auto &p : particles)
			p->update(DELTA_TIME);
	}
}

using P76Fix = ParticlesObjVectorFixture<Particle>;
BENCHMARK_DEFINE_F(P76Fix, Obj)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesObj(particles);
	}
}
BENCHMARK_REGISTER_F(P76Fix, Obj)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

using P160Fix = ParticlesObjVectorFixture<Particle160>;
BENCHMARK_DEFINE_F(P160Fix, Obj)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesObj(particles);
	}
}
BENCHMARK_REGISTER_F(P160Fix, Obj)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

/////////////////////////////////////////////////////////////////////
// shared_ptr
using P76FixShared = ParticlesPtrVectorFixture<Particle>;
BENCHMARK_DEFINE_F(P76FixShared, Shared)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesPtr(particles);
	}
}
BENCHMARK_REGISTER_F(P76FixShared, Shared)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

using P160FixShared = ParticlesPtrVectorFixture<Particle160>;
BENCHMARK_DEFINE_F(P160FixShared, Shared)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesPtr(particles);
	}
}
BENCHMARK_REGISTER_F(P160FixShared, Shared)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

/////////////////////////////////////////////////////////////////////
// shared_ptr no rand
using P76FixSharedNoRand = ParticlesPtrVectorNoRandFixture<Particle>;
BENCHMARK_DEFINE_F(P76FixSharedNoRand, Shared)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesPtr(particles);
	}
}
BENCHMARK_REGISTER_F(P76FixSharedNoRand, Shared)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

using P160FixSharedNoRand = ParticlesPtrVectorNoRandFixture<Particle160>;
BENCHMARK_DEFINE_F(P160FixSharedNoRand, Shared)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesPtr(particles);
	}
}
BENCHMARK_REGISTER_F(P160FixSharedNoRand, Shared)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

/////////////////////////////////////////////////////////////////////
// unique_ptr 
using P76FixUnique = ParticlesUniquePtrVectorFixture<Particle>;
BENCHMARK_DEFINE_F(P76FixUnique, Unique)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesPtr(particles);
	}
}
BENCHMARK_REGISTER_F(P76FixUnique, Unique)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

using P160FixUnique = ParticlesUniquePtrVectorFixture<Particle160>;
BENCHMARK_DEFINE_F(P160FixUnique, Unique)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesPtr(particles);
	}
}
BENCHMARK_REGISTER_F(P160FixUnique, Unique)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

/////////////////////////////////////////////////////////////////////
// unique_ptr no rand
using P76FixUniqueNoRand = ParticlesUniquePtrVectorFixtureNoRand<Particle>;
BENCHMARK_DEFINE_F(P76FixUniqueNoRand, Unique)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesPtr(particles);
	}
}
BENCHMARK_REGISTER_F(P76FixUniqueNoRand, Unique)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

using P160FixUniqueNoRand = ParticlesUniquePtrVectorFixtureNoRand<Particle160>;
BENCHMARK_DEFINE_F(P160FixUniqueNoRand, Unique)(benchmark::State& state) {
	while (state.KeepRunning()) {
		UpdateParticlesPtr(particles);
	}
}
BENCHMARK_REGISTER_F(P160FixUniqueNoRand, Unique)->Apply(CustomArguments)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN()