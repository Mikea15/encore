#include "animated_sprite.h"

AnimatedSprite::AnimatedSprite()
	: m_spritesheet(nullptr)
	  , m_currentAnimation(nullptr)
	  , m_currentFrameIndex(0)
	  , m_currentFrameTime(0.0f)
	  , m_playbackSpeed(1.0f)
	  , m_isPlaying(false)
	  , m_isFinished(false)
	  , m_defaultFrame(0.0f, 0.0f, 1.0f, 1.0f)
{
}

AnimatedSprite::AnimatedSprite(const Spritesheet* spritesheet)
	: m_spritesheet(spritesheet)
	  , m_currentAnimation(nullptr)
	  , m_currentFrameIndex(0)
	  , m_currentFrameTime(0.0f)
	  , m_playbackSpeed(1.0f)
	  , m_isPlaying(false)
	  , m_isFinished(false)
	  , m_defaultFrame(0.0f, 0.0f, 1.0f, 1.0f)
{
	if (m_spritesheet)
	{
		m_defaultFrame = m_spritesheet->GetTile(0, 0);
	}
}

void AnimatedSprite::SetSpritesheet(const Spritesheet* spritesheet)
{
	m_spritesheet = spritesheet;
	m_currentAnimation = nullptr;
	m_currentAnimationName.clear();
	ResetAnimation();

	if (m_spritesheet)
	{
		m_defaultFrame = m_spritesheet->GetTile(0, 0);
	}
}

bool AnimatedSprite::PlayAnimation(const std::string& animationName, bool restart)
{
	if (!m_spritesheet)
	{
		LOG_ERROR("No spritesheet set for animated sprite");
		return false;
	}

	const Animation* animation = m_spritesheet->GetAnimation(animationName);
	if (!animation)
	{
		LOG_ERROR("Animation '%s' not found", animationName.c_str());
		return false;
	}

	// If same animation is already playing and restart is false, do nothing
	if (m_currentAnimation == animation && m_isPlaying && !restart)
	{
		return true;
	}

	m_currentAnimation = animation;
	m_currentAnimationName = animationName;
	ResetAnimation();
	m_isPlaying = true;
	m_isFinished = false;

	LOG_INFO("Started animation '%s'", animationName.c_str());
	return true;
}

void AnimatedSprite::StopAnimation()
{
	m_isPlaying = false;
	m_isFinished = true;
	ResetAnimation();
}

void AnimatedSprite::PauseAnimation()
{
	m_isPlaying = false;
}

void AnimatedSprite::ResumeAnimation()
{
	if (m_currentAnimation && !m_isFinished)
	{
		m_isPlaying = true;
	}
}

void AnimatedSprite::Update(float deltaTime)
{
	if (!m_isPlaying || !m_currentAnimation || m_currentAnimation->frames.empty())
	{
		return;
	}

	m_currentFrameTime += deltaTime * m_playbackSpeed;

	const SpriteFrame& currentFrame = m_currentAnimation->frames[m_currentFrameIndex];

	if (m_currentFrameTime >= currentFrame.duration)
	{
		m_currentFrameTime -= currentFrame.duration;
		m_currentFrameIndex++;

		// Check if animation is complete
		if (m_currentFrameIndex >= m_currentAnimation->frames.size())
		{
			if (m_currentAnimation->loop)
			{
				m_currentFrameIndex = 0; // Loop back to start
			}
			else
			{
				m_currentFrameIndex = static_cast<u32>(m_currentAnimation->frames.size()) - 1;
				m_isPlaying = false;
				m_isFinished = true;
			}
		}
	}
}

const SpriteFrame& AnimatedSprite::GetCurrentFrame() const
{
	if (m_currentAnimation && !m_currentAnimation->frames.empty() &&
		m_currentFrameIndex < m_currentAnimation->frames.size())
	{
		return m_currentAnimation->frames[m_currentFrameIndex];
	}

	return m_defaultFrame;
}

void AnimatedSprite::SetFrame(u32 frameIndex)
{
	if (m_currentAnimation && frameIndex < m_currentAnimation->frames.size())
	{
		m_currentFrameIndex = frameIndex;
		m_currentFrameTime = 0.0f;
	}
}

u32 AnimatedSprite::GetFrameCount() const
{
	return m_currentAnimation ? static_cast<u32>(m_currentAnimation->frames.size()) : 0;
}

void AnimatedSprite::Bind(u32 textureUnit) const
{
	if (m_spritesheet)
	{
		m_spritesheet->Bind(textureUnit);
	}
}

void AnimatedSprite::Unbind() const
{
	if (m_spritesheet)
	{
		m_spritesheet->Unbind();
	}
}

void AnimatedSprite::ResetAnimation()
{
	m_currentFrameIndex = 0;
	m_currentFrameTime = 0.0f;
}
