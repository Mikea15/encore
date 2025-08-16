#pragma once

#include "core/core_minimal.h"
#include "memory/base_arena.h"

#if ENC_DEBUG
inline struct FrameStats
{
	f32* pFrameTimes = nullptr;
	f32* pFramesPerSecond = nullptr;
	f32 avgFps = 0.0f;
	f32 oneSec = 0.0f;
	u32 sampleCount = 120;
	u32 frameCount = 0;
	u32 framesOneSec = 0;
} g_frameStats {
	.avgFps = 0.f,
	.oneSec = 0.f,
	.sampleCount = 120,
	.frameCount = 0,
	.framesOneSec = 0
};

static void frame_stats_init(FrameStats& stats, Arena& arena)
{
	stats.pFrameTimes = arena_alloc_array(&arena, f32, stats.sampleCount);
	stats.pFramesPerSecond = arena_alloc_array(&arena, f32, stats.sampleCount);

	AssertMsg(stats.pFrameTimes, "Could not allocate, check arena usage. ");
	AssertMsg(stats.pFramesPerSecond, "Could not allocate, check arena usage. ");
}

static void frame_stats_reset(FrameStats& stats)
{
	stats.avgFps = 0.f;
	stats.oneSec = 0.f;
	stats.sampleCount = 120;
	stats.frameCount = 0;
	stats.framesOneSec = 0;
}

static void frame_stats_update(FrameStats& stats, float deltaTime)
{
	float current_fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;

	stats.frameCount++;
	stats.framesOneSec++;

	const u32 index = stats.frameCount % stats.sampleCount;

	// Add frametime sample
	stats.pFrameTimes[index] = deltaTime * 1000.0f;

	// Add FPS sample
	stats.pFramesPerSecond[index] = current_fps;

	// Update statistics
	stats.oneSec += deltaTime;
	if (stats.oneSec >= 1.0f)
	{
		stats.avgFps = static_cast<float>(stats.framesOneSec);

		stats.oneSec -= 1.0f;
		stats.framesOneSec = 0;
	}
}
#endif

