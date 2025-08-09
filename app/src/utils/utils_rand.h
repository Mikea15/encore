#pragma once

#include "core/core_minimal.h"
#include <random>

namespace utils 
{
	thread_local std::mt19937 engine(std::random_device{}());

	// Generate random integer in range [min, max]
	i32 GetInt(i32 min, i32 max)
	{
		std::uniform_int_distribution<i32> dist(min, max);
		return dist(engine);
	}

	// Generate seeded random integer in range [min, max]
	i32 GetInt_seeded(i32 min, i32 max, u32 seed)
	{
		std::mt19937 seeded_engine(seed);
		std::uniform_int_distribution<i32> dist(min, max);
		return dist(seeded_engine);
	}

	// Generate random f32 in range [min, max)
	f32 GetFloat(f32 min = 0.0f, f32 max = 1.0f)
	{
		std::uniform_real_distribution<f32> dist(min, max);
		return dist(engine);
	}

	// Generate seeded random f32 in range [min, max)
	f32 GetFloat_seeded(f32 min, f32 max, u32 seed)
	{
		std::mt19937 seeded_engine(seed);
		std::uniform_real_distribution<f32> dist(min, max);
		return dist(seeded_engine);
	}

	// Generate random f64 in range [min, max)
	f64 GetFloat(f64 min = 0.0, f64 max = 1.0)
	{
		std::uniform_real_distribution<f64> dist(min, max);
		return dist(engine);
	}

	// Generate seeded random f64 in range [min, max)
	f64 GetFloat_seeded(f64 min, f64 max, u32 seed)
	{
		std::mt19937 seeded_engine(seed);
		std::uniform_real_distribution<f64> dist(min, max);
		return dist(seeded_engine);
	}

	// Generate random f32 with normal distribution
	f32 GetNormalDist(f32 mean = 0.0f, f32 stddev = 1.0f)
	{
		std::normal_distribution<f32> dist(mean, stddev);
		return dist(engine);
	}

	// Generate seeded random f32 with normal distribution
	f32 GetNormalDist_seeded(f32 mean, f32 stddev, u32 seed)
	{
		std::mt19937 seeded_engine(seed);
		std::normal_distribution<f32> dist(mean, stddev);
		return dist(seeded_engine);
	}

	// Generate random boolean with given probability
	bool GetBool(f64 probability = 0.5)
	{
		std::bernoulli_distribution dist(probability);
		return dist(engine);
	}

	// Generate seeded random boolean with given probability
	bool GetBool(f64 probability, u32 seed)
	{
		std::mt19937 seeded_engine(seed);
		std::bernoulli_distribution dist(probability);
		return dist(seeded_engine);
	}

	void seed(u32 s)
	{
		engine.seed(s);
	}
}
