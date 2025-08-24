#pragma once

#include "core/core_minimal.h"

#include "state.h"
#include "assets/animated_sprite.h"
#include "assets/texture_manager.h"
#include "entity/entity.h"
#include "utils/utils_path.h"

enum SlimeState : u8
{
	ST_Walk,
	ST_Run,
	ST_Idle,
	ST_Hurt,
	ST_Death,
	ST_Attack,

	ST_Count
};

class SandboxState : public State
{
public:
	virtual void Init() override
	{
		LOG_INFO("Working Directory: %s", utils::GetCurrentWorkingDirectory());
		// Load a tileset spritesheet (e.g., 16x16 tiles)

		constexpr float kDefaultAnimationRate = 1.0f/24.0f; // 24 fps
		m_slimeStates[ST_Walk] = TextureManager::GetInstance().LoadSpritesheet(utils::GetPath("sprites/Slime1/Walk/Slime1_Walk_full.png"), 64, 64);
		m_slimeStates[ST_Walk].AddAnimation("Walk_F", 0, 0, 6, true, kDefaultAnimationRate, true);
		m_slimeStates[ST_Death] = TextureManager::GetInstance().LoadSpritesheet(utils::GetPath("sprites/Slime1/Death/Slime1_Death_full.png"), 64, 64);
		m_slimeStates[ST_Death].AddAnimation("Death_F", 0, 0, 10, true, kDefaultAnimationRate, true);
		m_slimeStates[ST_Attack] = TextureManager::GetInstance().LoadSpritesheet(utils::GetPath("sprites/Slime1/Attack/Slime1_Attack_full.png"), 64, 64);
		m_slimeStates[ST_Attack].AddAnimation("Attack_F", 0, 0, 10, true, kDefaultAnimationRate, true);

		m_spritesheet2 = TextureManager::GetInstance().LoadSpritesheet(utils::GetPath("sprites/Slime2/Idle/Slime2_Idle_body.png"), 64, 64);
		m_spritesheet2.AddAnimation("idle_front", 0, 0, 6, true, kDefaultAnimationRate, true);

		m_characterSprite1 = AnimatedSprite(&m_slimeStates[ST_Attack]);
		m_characterSprite1.PlayAnimation("Attack_F");

		m_characterSprite2 = AnimatedSprite(&m_spritesheet2);
		m_characterSprite2.PlayAnimation("idle_front");

		// Get specific tiles for level painting
		// SpriteFrame grassTile = tileset.GetTile(0, 0);    // First tile (top-left)
		// SpriteFrame stoneTile = tileset.GetTile(1, 0);    // Second tile in first row
		// SpriteFrame waterTile = tileset.GetTile(0, 1);    // First tile in second row

		// Create some test sprites
		for(int i = 0; i < 100000; ++i)
		{
			Vec2 position = {
				utils::GetFloat(-1500.0f, 1500.0f),
				utils::GetFloat(-1500.0f, 1500.0f)
			};
			// sprite.size = { utils::GetFloat(1.0f, 64.0f), utils::GetFloat(1.0f, 64.0f) };
			//sprite.color = {
			//	(rand() % 255) / 255.0f, // Random color
			//	(rand() % 255) / 255.0f,
			//	(rand() % 255) / 255.0f,
			//	1.0f
			//};
			// sprite.texId = texSlimeIdle;

			Entity* pEnt = Entity::Alloc();
			Assert(pEnt);
			pEnt->RegisterComponents(position, utils::GetFloat(0.0f, 360.0f), utils::GetBool() ? m_characterSprite1 : m_characterSprite2);
		}
	}

private:
	Spritesheet m_slimeStates[ST_Count];

	Spritesheet m_spritesheet2;

	AnimatedSprite m_characterSprite1;
	AnimatedSprite m_characterSprite2;
};

#if INC_DEMO
void ExampleUsage()
{
    TextureManager textureManager;

    // === LEVEL PAINTING EXAMPLE ===
    // Load a tileset spritesheet (e.g., 16x16 tiles)
    Spritesheet tileset = textureManager.LoadSpritesheet("assets/tileset.png", 16, 16);

    // Get specific tiles for level painting
    SpriteFrame grassTile = tileset.GetTile(0, 0);    // First tile (top-left)
    SpriteFrame stoneTile = tileset.GetTile(1, 0);    // Second tile in first row
    SpriteFrame waterTile = tileset.GetTile(0, 1);    // First tile in second row

    // Use these frames with your quad/sprite renderer
    // The SpriteFrame contains u1, v1, u2, v2 UV coordinates

    // === CHARACTER ANIMATION EXAMPLE ===
    // Load character spritesheet (e.g., 32x32 character frames)
    Spritesheet characterSheet = textureManager.LoadSpritesheet("assets/character.png", 32, 32);

    // Define animations
    // Walk animation: frames 0-3 in first row
    characterSheet.AddAnimation("walk_right", 0, 0, 4, true, 0.15f, true);

    // Idle animation: frames 0-1 in second row
    characterSheet.AddAnimation("idle", 0, 1, 2, true, 0.5f, true);

    // Attack animation: frames 0-2 in third row (non-looping)
    characterSheet.AddAnimation("attack", 0, 2, 3, true, 0.1f, false);

    // Alternative way to define animations using frame indices
    std::vector<u32> jumpFrames = {12, 13, 14, 15}; // Specific frame indices
    characterSheet.AddAnimation("jump", jumpFrames, 0.2f, false);

    // Create animated sprite
    AnimatedSprite character(&characterSheet);

    // Start idle animation
    character.PlayAnimation("idle");
}

// Example of creating a tile-based level renderer
class TileRenderer
{
private:
    Spritesheet m_tileset;

public:

    void RenderTile(u32 tileId, float x, float y, float size)
    {
        // Convert tile ID to column/row
        u32 column = tileId % m_tileset.GetColumns();
        u32 row = tileId / m_tileset.GetColumns();

        SpriteFrame tile = m_tileset.GetTile(column, row);

        m_tileset.Bind(0);
        RenderQuadAt(x, y, size, size, tile.u1, tile.v1, tile.u2, tile.v2);
        m_tileset.Unbind();
    }

private:
    void RenderQuadAt(float x, float y, float w, float h, float u1, float v1, float u2, float v2)
    {
        // Your quad rendering implementation here
        // This would typically set up vertex data and draw
    }
};
#endif
