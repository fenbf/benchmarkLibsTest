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

NONIUS_BENCHMARK("ParticlesStack", [](nonius::chronometer meter) {
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

NONIUS_BENCHMARK("ParticlesHeap", [](nonius::chronometer meter) {
	std::vector<std::shared_ptr<Particle>> particles(NUM_PARTICLES);
	for (auto &p : particles)
	{
		p = std::make_shared<Particle>();
	}

	for (size_t i = 0; i < NUM_PARTICLES / 2; ++i)
	{
		int a = rand() % NUM_PARTICLES;
		int b = rand() % NUM_PARTICLES;
		if (a != b)
			std::swap(particles[a], particles[b]);
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

NONIUS_BENCHMARK("ParticlesHeapNoRand", [](nonius::chronometer meter) {
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