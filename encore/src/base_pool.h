#pragma once

#include "core_minimal.h"

template<typename T>
class Pool {
private:
    T* items;
    u32* free_list;
    u32 capacity;
    u32 free_count;
    b8* active;

public:
    Pool() 
        : items(nullptr)
        , free_list(nullptr)
        , capacity(0)
        , free_count(0)
        , active(nullptr) 
    {}

    bool Init(Arena* arena, u32 cap) {
        if (items) return false;

        items = arena_alloc_array(arena, T, cap);
        free_list = arena_alloc_array(arena, u32, cap);
        active = arena_alloc_array(arena, b8, cap);

        if (!items || !free_list || !active) {
            return false;
        }

        capacity = cap;
        free_count = cap;

        for (u32 i = 0; i < cap; i++) {
            free_list[i] = i;
            active[i] = false;
        }

        return true;
    }

    T* Alloc() {
        if (!items || free_count == 0) return nullptr;

        u32 index = free_list[--free_count];
        active[index] = true;
        new(&items[index]) T(); // Placement new for proper construction
        return &items[index];
    }

    void Free(T* item) {
        if (!item || !items) return;

        u32 index = static_cast<u32>(item - items);
        if (index < capacity && active[index]) {
            active[index] = false;
            item->~T(); // Proper destruction
            free_list[free_count++] = index;
        }
    }

    u32 GetCapacity() const { return capacity; }
    u32 GetFreeCount() const { return free_count; }
    u32 GetActiveCount() const { return capacity - free_count; }

    // Iterator support
    class Iterator {
        const Pool* pool;
        u32 index;

        void FindNext() {
            while (index < pool->capacity && !pool->active[index]) {
                index++;
            }
        }

    public:
        Iterator(const Pool* p, u32 i) : pool(p), index(i) {
            FindNext();
        }

        T& operator*() { return pool->items[index]; }
        T* operator->() { return &pool->items[index]; }

        Iterator& operator++() {
            index++;
            FindNext();
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }
    };

    Iterator begin() const { return Iterator(this, 0); }
    Iterator end() const { return Iterator(this, capacity); }
};

// Pool macros
#define DECLARE_POOL(Type) \
    static Pool<Type> pool; \
    static Pool<Type>* GetPool(); \
    static Type* Alloc(); \
    static void Free(Type* item); \
    static bool Init(Arena* arena);

#define IMPLEMENT_POOL(Type, Cap) \
    Pool<Type> Type::pool; \
    Pool<Type>* Type::GetPool() { return &pool; } \
    Type* Type::Alloc() { return pool.Alloc(); } \
    void Type::Free(Type* item) { pool.Free(item); } \
    bool Type::Init(Arena* arena) { return pool.Init(arena, Cap); }

// Example usage
struct Particle {
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
    if (!Particle::Init(arena)) {
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
    for (auto& particle : *pool) {
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
    for (const auto& particle : *Particle::GetPool()) {
        printf("Particle at x=%.0f, lifetime=%.1f\n", particle.x, particle.lifetime);
    }
}