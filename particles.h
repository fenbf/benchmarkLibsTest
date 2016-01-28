#pragma once

static int randSeed = 10;
inline float randF() { return 0.01f*(float)((randSeed++) % 100); }// (float)rand() / (float)RAND_MAX; }

static const size_t UPDATES = 1000;
static const float DELTA_TIME = 1.0f / 60.0f;

class Particle
{
private:
	float pos[4];
	float acc[4];
	float vel[4];
	float col[4];
	float rot;
	float time;
public:

	void generate()
	{
		acc[0] = randF();
		acc[1] = randF();
		acc[2] = randF();
		acc[3] = randF();
		pos[0] = pos[1] = pos[2] = pos[3] = 0.0f;
		vel[0] = randF();
		vel[1] = randF();
		vel[2] = randF();
		vel[3] = vel[1] + vel[2];
		rot = 0.0f;
		time = 2.0f+randF();
	}

	void update(float dt)
	{
		vel[0] += acc[0] * dt;
		vel[1] += acc[1] * dt;
		vel[2] += acc[2] * dt;
		vel[3] += acc[3] * dt;
		pos[0] += vel[0] * dt;
		pos[1] += vel[1] * dt;
		pos[2] += vel[2] * dt;
		pos[3] += vel[3] * dt;
		col[0] = pos[0] * 0.001f;
		col[1] = pos[1] * 0.001f;
		col[2] = pos[2] * 0.001f;
		col[3] = pos[3] * 0.001f;
		rot += vel[3] * dt;
		time -= dt;

		if (time < 0.0f)
			generate();
	}
};