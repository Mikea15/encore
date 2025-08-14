#pragma once

#include "core/core_minimal.h"

// Mathematical constants
const f32 PI = 3.14159265358979323846f;
const f32 TAU = 6.28318530717958647692f;
const f32 E = 2.71828182845904523536f;

const f64 PI_D = 3.14159265358979323846;
const f64 TAU_D = 6.28318530717958647692;
const f64 E_D = 2.71828182845904523536;

const f64 SMALL_NUMBER = 1e-6f;
const f64 TINY_NUMBER = 1e-4f;

namespace utils 
{
	template<typename T>
	inline T Min(T a, T b) { return (a < b) ? a : b; }

	template<typename T>
	inline T Max(T a, T b) { return (a > b) ? a : b; }

	template<typename T>
	inline T Clamp(T value, T min, T max)
	{
		return Min(Max(value, min), max);
	}

	template<typename T>
	inline T Clamp01(T value)
	{
		return Min(Max(value, (T)0), (T)1);
	}

	template<typename T>
	inline T Lerp(T a, T b, T t)
	{
		return a + (b - a) * t;
	}

	template<typename T>
	inline T InvLerp(T a, T b, T value)
	{
		return (value - a) / (b - a);
	}

	// Smoothstep interpolation
	template<typename T>
	inline T SmoothStep(T edge0, T edge1, T x)
	{
		T t = Clamp01((x - edge0) / (edge1 - edge0));
		return t * t * (3.0f - 2.0f * t);
	}

	template<typename T>
	inline T Abs(T value)
	{
		return (value < (T)0) ? -value : value;
	}

	template<typename T>
	inline T Sign(T value)
	{
		return ((T)0 < value) - (value < 0.0f);
	}

	template<typename T>
	inline T Square(T value)
	{
		return value * value;
	}

	template<typename T>
	inline T Map(T value, T in_min, T in_max, T out_min, T out_max)
	{
		return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
	}

	// Wrap a value to a range [min, max)
	inline f32 Wrap(f32 value, f32 min, f32 max)
	{
		f32 range = max - min;
		if(range <= 0.0f) return min;

		f32 result = value - min;
		result = result - std::floor(result / range) * range;
		return result + min;
	}

	// Wrap a value to a range [min, max)
	inline f64 Wrap(f64 value, f64 min, f64 max)
	{
		f64 range = max - min;
		if(range <= 0.0) return min;

		f64 result = value - min;
		result = result - std::floor(result / range) * range;
		return result + min;
	}

	inline f32 Radians(f32 degrees)
	{
		return degrees * PI / 180.0f;
	}

	inline f64 Radians(f64 degrees)
	{
		return degrees * PI_D / 180.0;
	}

	inline f32 Degrees(f32 radians)
	{
		return radians * 180.0f / PI;
	}

	inline f64 Degrees(f64 radians)
	{
		return radians * 180.0 / PI_D;
	}

	inline bool Equals(f32 a, f32 b, f32 epsilon = 1e-6f)
	{
		return abs(a - b) < epsilon;
	}

	inline bool Equals(f64 a, f64 b, f64 epsilon = 1e-9)
	{
		return abs(a - b) < epsilon;
	}

	// Step function (returns 0 if x < edge, 1 otherwise)
	inline f32 Step(f32 edge, f32 x)
	{
		return x < edge ? 0.0f : 1.0f;
	}

	inline f64 Step(f64 edge, f64 x)
	{
		return x < edge ? 0.0 : 1.0;
	}

	// Fractional part
	inline f32 Fract(f32 value)
	{
		return value - std::floor(value);
	}

	inline f64 Fract(f64 value)
	{
		return value - std::floor(value);
	}
}
