#pragma once

#include "core/core_minimal.h"
#include "base_arena.h"

#include <utility>
#include <type_traits>

#define COMPILE_DEMO 0

class PoolId
{
	template<typename T> friend class Pool;

public:
	PoolId() : m_id(INVALID_U32) {}
	virtual ~PoolId() = default;

	u32 GetId() const { return m_id; }
	bool IsValid() const { return m_id != INDEX_NONE; }

protected:
	u32 m_id;
};

template<typename T>
class Pool
{
	static_assert(std::is_base_of<PoolId, T>::value, "Type T must inherit from PoolId");

public:
	Pool()
		: m_pItems(nullptr)
		, m_pFreeList(nullptr)
		, m_capacity(0)
		, m_freeCount(0)
		, m_pActive(nullptr)
		, m_bWarningLogged(false)
	{}

	bool Init(Arena* pArena, u32 capacity)
	{
		if(m_pItems)
		{
			LOG_ERROR("Pool already initialized");
			return false;
		}

		EnsureMsg(pArena != nullptr, "Arena cannot be null");
		EnsureMsg(capacity > 0, "Pool capacity must be greater than 0");

		m_pItems = arena_alloc_array(pArena, T, capacity);
		m_pFreeList = arena_alloc_array(pArena, u32, capacity);
		m_pActive = arena_alloc_array(pArena, b8, capacity);

		if(!m_pItems || !m_pFreeList || !m_pActive)
		{
			LOG_ERROR("Failed to allocate memory for pool (capacity: %u, size per item: %zu bytes, total: %zu bytes)",
				capacity, sizeof(T), capacity * (sizeof(T) + sizeof(u32) + sizeof(b8)));
			return false;
		}

		m_capacity = capacity;
		m_freeCount = capacity;
		m_bWarningLogged = false;

		// Initialize free list and active flags
		for(u32 i = 0; i < capacity; i++)
		{
			m_pFreeList[i] = i;
			m_pActive[i] = false;
		}

		LOG_INFO("Pool initialized successfully (type: %s, capacity: %u, total memory: %zu bytes)",
			typeid(T).name(), capacity, capacity * (sizeof(T) + sizeof(u32) + sizeof(b8)));

		return true;
	}

	template<typename... Args>
	T* Alloc(Args&&... args)
	{
		if(!m_pItems)
		{
			LOG_ERROR("Pool not initialized - cannot allocate");
			return nullptr;
		}

		if(m_freeCount == 0)
		{
			LOG_ERROR("Pool exhausted - no free slots available (capacity: %u)", m_capacity);
			return nullptr;
		}

		// Check if we're approaching capacity limit (70% used)
		u32 usedCount = m_capacity - m_freeCount;
		float usagePercentage = (float)usedCount / (float)m_capacity * 100.0f;

		if(!m_bWarningLogged && usagePercentage >= 70.0f)
		{
			LOG_WARNING("Pool approaching capacity limit: %.1f%% used (%u/%u slots)",
				usagePercentage, usedCount, m_capacity);
			m_bWarningLogged = true;
		}

		u32 index = m_pFreeList[--m_freeCount];
		AssertMsg(index < m_capacity, "Invalid free list index");
		AssertMsg(!m_pActive[index], "Slot should be inactive before allocation");

		m_pActive[index] = true;

		// Use placement new with perfect forwarding for proper construction
		T* newItem = new(&m_pItems[index]) T(std::forward<Args>(args)...);

		static_cast<PoolId*>(newItem)->m_id = index;

		return &m_pItems[index];
	}

	T* Get(u32 id)
	{
		AssertMsg(id >= 0, "Trying to provide an invalid id:");

		if(id >= m_capacity)
		{
			LOG_WARNING("Id over capacity!");
			return nullptr;
		}
		if(!m_pActive[id])
		{
			LOG_WARNING("Object not active. Was not allocated properly!");
			return nullptr;
		}
		return &m_pItems[id];
	}

	const T* Get(u32 id) const
	{
		AssertMsg(id >= 0, "Trying to provide an invalid id");

		if(id >= m_capacity)
		{
			LOG_WARNING("Id over capacity!");
			return nullptr;
		}
		if(!m_pActive[id])
		{
			LOG_WARNING("Object not active. Was not allocated properly!");
			return nullptr;
		}
		return &m_pItems[id];
	}

	void Free(T* pItem)
	{
		if(!pItem)
		{
			LOG_WARNING("Attempted to free null pointer");
			return;
		}

		AssertMsg(m_pItems, "Pool not initialized! No Items to free");

		// Validate item is within pool bounds
		u32 index = static_cast<u32>(pItem - m_pItems);
		if(index >= m_capacity)
		{
			LOG_ERROR("Item pointer outside pool bounds (index: %u, capacity: %u)", index, m_capacity);
			return;
		}

		if(!m_pActive[index])
		{
			LOG_ERROR("Attempted to free already inactive slot (index: %u)", index);
			return;
		}

		// Proper destruction and cleanup
		m_pActive[index] = false;
		pItem->~T();

		AssertMsg(m_freeCount < m_capacity, "Free count would exceed capacity");
		m_pFreeList[m_freeCount++] = index;

		// Reset warning flag if we drop below 70% usage
		u32 usedCount = m_capacity - m_freeCount;
		float usagePercentage = (float)usedCount / (float)m_capacity * 100.0f;
		if(m_bWarningLogged && usagePercentage < 70.0f)
		{
			m_bWarningLogged = false;
		}
	}

	void Free(u32 id)
	{
		T* item = Get(id);
		Free(item);
	}

	u32 GetCapacity() const { return m_capacity; }
	u32 GetFreeCount() const { return m_freeCount; }
	u32 GetActiveCount() const { return m_capacity - m_freeCount; }

	float GetUsagePercentage() const
	{
		return m_capacity > 0 ? ((float)(m_capacity - m_freeCount) / (float)m_capacity * 100.0f) : 0.0f;
	}

	// Iterator support
	class Iterator
	{
	public:
		Iterator(const Pool* pPool, u32 startIndex) : m_pPool(pPool), m_index(startIndex)
		{
			FindNext();
		}

		T& operator*() { return m_pPool->m_pItems[m_index]; }
		T* operator->() { return &m_pPool->m_pItems[m_index]; }

		Iterator& operator++()
		{
			m_index++;
			FindNext();
			return *this;
		}

		bool operator!=(const Iterator& rOther) const
		{
			return m_index != rOther.m_index;
		}

	private:
		void FindNext()
		{
			while(m_index < m_pPool->m_capacity && !m_pPool->m_pActive[m_index])
			{
				m_index++;
			}
		}

		const Pool* m_pPool;
		u32 m_index;
	};

	Iterator begin() const { return Iterator(this, 0); }
	Iterator end() const { return Iterator(this, m_capacity); }

private:
	T* m_pItems;
	u32* m_pFreeList;
	u32 m_capacity;
	u32 m_freeCount;
	b8* m_pActive;
	b8 m_bWarningLogged;  // Track if we've already logged the 70% warning
};

// Pool macros
#define DECLARE_POOL(Type) \
    static Pool<Type> pool; \
    static Pool<Type>* GetPool() { return &pool; } \
	template<typename... Args> \
    static Type* Alloc(Args&&... args) { return pool.Alloc(std::forward<Args>(args)...); } \
    static void Free(Type* pItem) { pool.Free(pItem); } \
    static void Free(u32 id) { pool.Free(id); } \
    static bool Init(Arena* pArena);

#define IMPLEMENT_POOL(Type, Cap) \
    Pool<Type> Type::pool; \
    bool Type::Init(Arena* pArena) { return pool.Init(pArena, Cap); }

#if COMPILE_DEMO
// Example usage
struct Particle
{
	DECLARE_POOL(Particle);

	Particle() : m_x(0), m_y(0), m_vx(0), m_vy(0), m_lifetime(0) {}
	Particle(float x, float y, float lifetime) : m_x(x), m_y(y), m_vx(0), m_vy(0), m_lifetime(lifetime) {}

private:
	float m_x, m_y;
	float m_vx, m_vy;
	float m_lifetime;
};

IMPLEMENT_POOL(Particle, 500);

void TestPool(Arena* pArena)
{
	// Initialize the pool
	if(!Particle::Init(pArena))
	{
		LOG_ERROR("Failed to initialize particle pool");
		return;
	}

	Pool<Particle>* pPool = Particle::GetPool();
	LOG_INFO("Pool test started. Initial free slots: %u (%.1f%% usage)",
		pPool->GetFreeCount(), pPool->GetUsagePercentage());

	// Allocate particles to trigger warning
	for(u32 i = 0; i < 400; i++)  // This should trigger the 70% warning
	{
		Particle* pParticle = Particle::Alloc(i * 10.0f, i * 5.0f, i * 0.1f);
		if(!pParticle)
		{
			LOG_ERROR("Failed to allocate particle %u", i);
			break;
		}
	}

	LOG_INFO("Allocated particles. Usage: %.1f%% (%u/%u)",
		pPool->GetUsagePercentage(), pPool->GetActiveCount(), pPool->GetCapacity());
}

#endif
