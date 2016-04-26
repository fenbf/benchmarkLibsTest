#define NONIUS_RUNNER
//#define NONIUS_USE_BOOST_CHRONO
#include "nonius-1.1.2\nonius\nonius.h++"
#include "../particles.h"

const size_t NUM_PARTICLES = 10000;

NONIUS_BENCHMARK("Test", [](nonius::chronometer meter) {
	// setup here

	meter.measure([] {
		// computation...
	});
});

NONIUS_BENCHMARK("ParticlesObjVector", [](nonius::chronometer meter) {
	std::vector<Particle> particles(NUM_PARTICLES);

	for (auto &p : particles)
		p.generate();

	meter.measure([&particles] { 
		for (size_t u = 0; u < UPDATES; ++u)
		{
			for (auto &p : particles)
				p.update(DELTA_TIME);
		}
	});
});

template <typename T>void RandomizePtrs(std::vector<T> *pVec)
{
	const size_t count = pVec->size() / 2;
	for (size_t i = 0; i < count; ++i)
	{
		int a = rand() % count;
		int b = rand() % count;
		if (a != b)
			std::swap((*pVec)[a], (*pVec)[b]);
	}
}

NONIUS_BENCHMARK("ParticlesPtr", [](nonius::chronometer meter) {
	std::vector<std::shared_ptr<Particle>> particles(NUM_PARTICLES);
	for (auto &p : particles)
	{
		p = std::make_shared<Particle>();
	}

	RandomizePtrs(&particles);

	for (auto &p : particles)
		p->generate();

	meter.measure([&particles] {
		for (size_t u = 0; u < UPDATES; ++u)
		{
			for (auto &p : particles)
				p->update(DELTA_TIME);
		}
	});
});

NONIUS_BENCHMARK("ParticlesPtrNoRand", [](nonius::chronometer meter) {
	std::vector<std::shared_ptr<Particle>> particles(NUM_PARTICLES);
	for (auto &p : particles)
	{
		p = std::make_shared<Particle>();
	}

	for (auto &p : particles)
		p->generate();

	meter.measure([&particles] {
		for (size_t u = 0; u < UPDATES; ++u)
		{
			for (auto &p : particles)
				p->update(DELTA_TIME);
		}
	});
});

NONIUS_BENCHMARK("ParticlesUniquePtr", [](nonius::chronometer meter) {
	std::vector<std::unique_ptr<Particle>> particles(NUM_PARTICLES);
	for (auto &p : particles)
	{
		p = std::make_unique<Particle>();
	}

	RandomizePtrs(&particles);

	for (auto &p : particles)
		p->generate();

	meter.measure([&particles] {
		for (size_t u = 0; u < UPDATES; ++u)
		{
			for (auto &p : particles)
				p->update(DELTA_TIME);
		}
	});
});

NONIUS_BENCHMARK("ParticlesUniquePtrNoRand", [](nonius::chronometer meter) {
	std::vector<std::unique_ptr<Particle>> particles(NUM_PARTICLES);
	for (auto &p : particles)
	{
		p = std::make_unique<Particle>();
	}

	for (auto &p : particles)
		p->generate();

	meter.measure([&particles] {
		for (size_t u = 0; u < UPDATES; ++u)
		{
			for (auto &p : particles)
				p->update(DELTA_TIME);
		}
	});

});