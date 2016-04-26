#include "celero\Celero.h"
#include "../particles.h"

CELERO_MAIN;

class ParticlesFixture : public celero::TestFixture
{
public:
	virtual std::vector<std::pair<int64_t, uint64_t>> getExperimentValues() const override
	{
		std::vector<std::pair<int64_t, uint64_t>> problemSpace;

		const int totalNumberOfTests = 10;

		for (int i = 0; i < totalNumberOfTests; i++)
		{
			// ExperimentValues is part of the base class and allows us to specify
			// some values to control various test runs to end up building a nice graph.
			problemSpace.push_back(std::make_pair(1000 + i * 1000, uint64_t(i+1)));
		}

		return problemSpace;
	}
};

class ParticlesObjVectorFixture : public ParticlesFixture
{
public:
	/// Before each run, build a vector of random integers.
	virtual void setUp(int64_t experimentValue) override
	{
		particles = std::vector<Particle>(experimentValue);

		for (auto &p : particles)
			p.generate();
	}

	/// After each run, clear the vector
	virtual void tearDown()
	{
		this->particles.clear();
	}

	std::vector<Particle> particles;
};

// Run an automatic baseline.  
// Celero will help make sure enough samples are taken to get a reasonable measurement
BASELINE_F(ParticlesTest, ObjVector, ParticlesObjVectorFixture, 20, 1)
{
	for (size_t u = 0; u < UPDATES; ++u)
	{
		for (auto &p : particles)
			p.update(DELTA_TIME);
	}
}

class ParticlesPtrVectorFixture : public ParticlesFixture
{
public:
	virtual bool randomizeAddresses() { return true; }

	/// Before each run, build a vector of random integers.
	virtual void setUp(int64_t experimentValue) override
	{
		particles = std::vector<std::shared_ptr<Particle>>(experimentValue);

		for (auto &p : particles)
			p = std::make_shared<Particle>();

		if (randomizeAddresses())
		{
			for (size_t i = 0; i < experimentValue / 2; ++i)
			{
				int a = rand() % experimentValue;
				int b = rand() % experimentValue;
				if (a != b)
					std::swap(particles[a], particles[b]);
			}
		}

		for (auto &p : particles)
			p->generate();
	}

	/// After each run, clear the vector
	virtual void tearDown()
	{
		this->particles.clear();
	}

	std::vector<std::shared_ptr<Particle>> particles;
};

BENCHMARK_F(ParticlesTest, PtrVector, ParticlesPtrVectorFixture, 20, 1)
{
	for (size_t u = 0; u < UPDATES; ++u)
	{
		for (auto &p : particles)
			p->update(DELTA_TIME);
	}
}

class ParticlesPtrVectorNoRandFixture : public ParticlesPtrVectorFixture
{
public:
	virtual bool randomizeAddresses() { return false; }
};

BENCHMARK_F(ParticlesTest, PtrVectorNoRand, ParticlesPtrVectorNoRandFixture, 20, 1)
{
	for (size_t u = 0; u < UPDATES; ++u)
	{
		for (auto &p : particles)
			p->update(DELTA_TIME);
	}
}