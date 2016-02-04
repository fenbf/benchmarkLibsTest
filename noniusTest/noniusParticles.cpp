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

NONIUS_BENCHMARK("ParticlesPtr", [](nonius::chronometer meter) {
	for (int count = 1000; count < 10000; count += 1000)
	{
		std::vector<std::shared_ptr<Particle>> particles(count);
		for (auto &p : particles)
		{
			p = std::make_shared<Particle>();
		}

		for (size_t i = 0; i < count / 2; ++i)
		{
			int a = rand() % count;
			int b = rand() % count;
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
	}
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