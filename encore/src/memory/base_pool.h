#pragma once

#include "core/core_minimal.h"
#include "base_arena.h"

#include <utility>

#define COMPILE_DEMO 0

template<typename T>
class Pool
{
public:
	Pool()
		: m_pItems(nullptr)
		, free_list(nullptr)
		, capacity(0)
		, m_freeCount(0)
		, active(nullptr)
	{}

	bool Init(Arena* arena, u32 cap)
	{
		if(m_pItems) return false;

		m_pItems = arena_alloc_array(arena, T, cap);
		free_list = arena_alloc_array(arena, u32, cap);
		active = arena_alloc_array(arena, b8, cap);

		if(!m_pItems || !free_list || !active)
		{
			return false;
		}

		capacity = cap;
		m_freeCount = cap;

		for(u32 i = 0; i < cap; i++)
		{
			free_list[i] = i;
			active[i] = false;
		}

		return true;
	}

	template<typename... Args>
	T* Alloc(Args&&... args)
	{
		if(!m_pItems)
		{
			LOG_ERROR("Pool not Initialized");
			return nullptr;
		}
		if(m_freeCount == 0)
		{
			LOG_ERROR("Pool full");
			return nullptr;
		}

		u32 index = free_list[--m_freeCount];
		active[index] = true;
		new(&m_pItems[index]) T(std::forward<Args>(args)...); // Placement new for proper construction
		return &m_pItems[index];
	}

	void Free(T* item)
	{
		if(!item || !m_pItems) return;

		u32 index = static_cast<u32>(item - m_pItems);
		if(index < capacity && active[index])
		{
			active[index] = false;
			item->~T(); // Proper destruction
			free_list[m_freeCount++] = index;
		}
	}

	u32 GetCapacity() const { return capacity; }
	u32 GetFreeCount() const { return m_freeCount; }
	u32 GetActiveCount() const { return capacity - m_freeCount; }

	// Iterator support
	class Iterator
	{
		const Pool* pool;
		u32 index;

		void FindNext()
		{
			while(index < pool->capacity && !pool->active[index])
			{
				index++;
			}
		}

	public:
		Iterator(const Pool* p, u32 i) : pool(p), index(i)
		{
			FindNext();
		}

		T& operator*() { return pool->m_pItems[index]; }
		T* operator->() { return &pool->m_pItems[index]; }

		Iterator& operator++()
		{
			index++;
			FindNext();
			return *this;
		}

		bool operator!=(const Iterator& other) const
		{
			return index != other.index;
		}
	};

	Iterator begin() const { return Iterator(this, 0); }
	Iterator end() const { return Iterator(this, capacity); }

private:
	T* m_pItems;
	u32* free_list;
	u32 capacity;
	u32 m_freeCount;
	b8* active;
};

// Pool macros
#define DECLARE_POOL(Type) \
    static Pool<Type> pool; \
    static Pool<Type>* GetPool(); \
	template<typename... Args> \
    static Type* Alloc(Args&&... args) { return pool.Alloc(std::forward<Args>(args)...); } \
    static void Free(Type* item); \
    static bool Init(Arena* arena);

#define IMPLEMENT_POOL(Type, Cap) \
    Pool<Type> Type::pool; \
    Pool<Type>* Type::GetPool() { return &pool; } \
    void Type::Free(Type* item) { pool.Free(item); } \
    bool Type::Init(Arena* arena) { return pool.Init(arena, Cap); }

#if COMPILE_DEMO
// Example usage
struct Particle
{
	DECLARE_POOL(Particle);

	float x, y;
	float vx, vy;
	float lifetime;

	Particle() : x(0), y(0), vx(0), vy(0), lifetime(0) {}
};

IMPLEMENT_POOL(Particle, 500);

void test_pool(Arena* arena)
{
	// Initialize the pool
	if(!Particle::Init(arena))
	{
		printf("Failed to init pool\n");
		return;
	}

	Pool<Particle>* pool = Particle::GetPool();
	printf("Pool initialized. Free slots: %u\n", pool->GetFreeCount());

	// Allocate particles using static methods
	Particle* p1 = Particle::Alloc();
	Particle* p2 = Particle::Alloc();
	Particle* p3 = Particle::Alloc();

	p1->x = 10; p1->lifetime = 1.0f;
	p2->x = 20; p2->lifetime = 2.0f;
	p3->x = 30; p3->lifetime = 3.0f;

	printf("Allocated 3 particles. Free slots: %u\n", pool->GetFreeCount());

	// Show particles using range-based for loop
	for(auto& particle : *pool)
	{
		printf("Particle at x=%.0f, lifetime=%.1f\n", particle.x, particle.lifetime);
	}

	// Free middle particle
	Particle::Free(p2);
	printf("Freed p2. Free slots: %u\n", pool->GetFreeCount());

	// Allocate new one (should reuse p2's slot)
	Particle* p4 = Particle::Alloc();
	p4->x = 99; p4->lifetime = 4.0f;
	printf("Allocated p4. Free slots: %u\n", pool->GetFreeCount());

	// Show final state
	printf("Final particles:\n");
	for(const auto& particle : *Particle::GetPool())
	{
		printf("Particle at x=%.0f, lifetime=%.1f\n", particle.x, particle.lifetime);
	}
}

#endif
