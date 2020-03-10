#include <pch.h>
#include "Random.h"
#include <random>

std::mt19937& random()
{
	static std::random_device seed;
#ifdef _DEBUG
	static std::mt19937 rng(1337);
#else
	static std::mt19937 rng(seed());
#endif // _DEBUG
	return rng;
}

float Tools::RandomNormalized()
{
	static std::uniform_real_distribution<float> dist(0.0f, 1.f);
	return dist(random());
}

size_t Tools::RandomRange(size_t aMin, size_t aMax)
{
	std::uniform_int_distribution<int> dist(aMin, aMax);
	return dist(random());
}

int Tools::RandomRange(int aMin, int aMax)
{
	std::uniform_int_distribution<int> dist(aMin, aMax);
	return dist(random());
}

float Tools::RandomRange(float aMin, float aMax)
{
	std::uniform_real_distribution<float> dist(aMin,aMax);
	return dist(random());
}
