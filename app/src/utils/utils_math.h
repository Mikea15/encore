#pragma once

#include "core/core_minimal.h"

// Mathematical constants
const f32 PI = 3.14159265358979323846f;
const f32 TAU = 6.28318530717958647692f;
const f32 E = 2.71828182845904523536f;

const f64 PI_D = 3.14159265358979323846;
const f64 TAU_D = 6.28318530717958647692;
const f64 E_D = 2.71828182845904523536;

namespace utils 
{
	inline f32 Min(f32 a, f32 b)
	{
		return (a < b) ? a : b;
	}

	inline f32 Max(f32 a, f32 b)
	{
		return (a > b) ? a : b;
	}

	inline f64 Min(f64 a, f64 b)
	{
		return (a < b) ? a : b;
	}

	inline f64 Max(f64 a, f64 b)
	{
		return (a > b) ? a : b;
	}

	inline i32 Min(i32 a, i32 b)
	{
		return (a < b) ? a : b;
	}

	inline i32 Max(i32 a, i32 b)
	{
		return (a > b) ? a : b;
	}

	inline f32 Clamp(f32 value, f32 min_val, f32 max_val)
	{
		return Min(Max(value, min_val), max_val);
	}

	inline f64 Clamp(f64 value, f64 min_val, f64 max_val)
	{
		return Min(Max(value, min_val), max_val);
	}

	inline i32 Clamp(i32 value, i32 min_val, i32 max_val)
	{
		return Min(Max(value, min_val), max_val);
	}

	inline f32 Clamp01(f32 value)
	{
		return Clamp(value, 0.0f, 1.0f);
	}

	inline f64 Clamp01(f64 value)
	{
		return Clamp(value, 0.0, 1.0);
	}

	inline f32 Lerp(f32 a, f32 b, f32 t)
	{
		return a + (b - a) * t;
	}

	inline f64 Lerp(f64 a, f64 b, f64 t)
	{
		return a + (b - a) * t;
	}

	inline f32 InvLerp(f32 a, f32 b, f32 value)
	{
		return (value - a) / (b - a);
	}

	inline f64 InvLerp(f64 a, f64 b, f64 value)
	{
		return (value - a) / (b - a);
	}

	// Smoothstep interpolation
	inline f32 SmoothStep(f32 edge0, f32 edge1, f32 x)
	{
		f32 t = Clamp01((x - edge0) / (edge1 - edge0));
		return t * t * (3.0f - 2.0f * t);
	}

	inline f64 SmoothStep(f64 edge0, f64 edge1, f64 x)
	{
		f64 t = Clamp01((x - edge0) / (edge1 - edge0));
		return t * t * (3.0 - 2.0 * t);
	}

	inline f32 Abs(f32 value)
	{
		return (value < 0.0f) ? -value : value;
	}

	inline f64 Abs(f64 value)
	{
		return (value < 0.0) ? -value : value;
	}

	inline i32 Abs(i32 value)
	{
		return (value < 0) ? -value : value;
	}

	inline f32 Sign(f32 value)
	{
		return (0.0f < value) - (value < 0.0f);
	}

	inline f64 Sign(f64 value)
	{
		return (0.0 < value) - (value < 0.0);
	}

	inline i32 Sign(i32 value)
	{
		return (0 < value) - (value < 0);
	}

	inline f32 Square(f32 value)
	{
		return value * value;
	}

	inline f64 Square(f64 value)
	{
		return value * value;
	}

	inline i32 Square(i32 value)
	{
		return value * value;
	}

	inline f32 Map(f32 value, f32 in_min, f32 in_max, f32 out_min, f32 out_max)
	{
		return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
	}

	inline f64 Map(f64 value, f64 in_min, f64 in_max, f64 out_min, f64 out_max)
	{
		return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
	}

	// Wrap a value to a range [min, max)
	inline f32 Wrap(f32 value, f32 min_val, f32 max_val)
	{
		f32 range = max_val - min_val;
		if(range <= 0.0f) return min_val;

		f32 result = value - min_val;
		result = result - std::floor(result / range) * range;
		return result + min_val;
	}

	// Wrap a value to a range [min, max)
	inline f64 Wrap(f64 value, f64 min_val, f64 max_val)
	{
		f64 range = max_val - min_val;
		if(range <= 0.0) return min_val;

		f64 result = value - min_val;
		result = result - std::floor(result / range) * range;
		return result + min_val;
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
