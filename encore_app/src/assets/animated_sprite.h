#pragma once

#include "core/core_minimal.h"


#include <string>

#include "sprite_sheet.h"

class AnimatedSprite
{
public:
	AnimatedSprite();
	AnimatedSprite(const Spritesheet* spritesheet);

	// Set the spritesheet to use
	void SetSpritesheet(const Spritesheet* spritesheet);

	// Animation control
	bool PlayAnimation(const std::string& animationName, bool restart = false);
	void StopAnimation();
	void PauseAnimation();
	void ResumeAnimation();

	// Update animation (call this each frame)
	void Update(float deltaTime);

	// Get current frame for rendering
	const SpriteFrame& GetCurrentFrame() const;

	// State queries
	bool IsPlaying() const { return m_isPlaying; }
	bool IsFinished() const { return m_isFinished; }
	const std::string& GetCurrentAnimationName() const { return m_currentAnimationName; }

	// Frame control
	void SetFrame(u32 frameIndex);
	u32 GetCurrentFrameIndex() const { return m_currentFrameIndex; }
	u32 GetFrameCount() const;

	// Speed control
	void SetPlaybackSpeed(float speed) { m_playbackSpeed = speed; }
	float GetPlaybackSpeed() const { return m_playbackSpeed; }

	// Rendering
	void Bind(u32 textureUnit = 0) const;
	void Unbind() const;

	const Spritesheet* GetSpritesheet() const { return m_spritesheet; }
	GLuint GetTextureID() const
	{
		return m_spritesheet ? m_spritesheet->GetTexture().GetTextureID() : 0;
	}

	struct FrameData
	{
		SpriteFrame frame;
		GLuint textureId;
	};

	FrameData GetCurrentFrameData() const
	{
		return { GetCurrentFrame(), GetTextureID() };
	}

private:
	const Spritesheet* m_spritesheet;
	const Animation* m_currentAnimation;
	std::string m_currentAnimationName;

	u32 m_currentFrameIndex;
	float m_currentFrameTime;
	float m_playbackSpeed;

	bool m_isPlaying;
	bool m_isFinished;

	// Default frame for when no animation is set
	SpriteFrame m_defaultFrame;

	void ResetAnimation();
};
