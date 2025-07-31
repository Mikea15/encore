#pragma once

#include "core_minimal.h"
#include <cstdlib>

typedef struct Arena {
    u8* memory;        // Pointer to the memory block
    u32 size;          // Total size of the arena
    u32 offset;        // Current offset (bytes used)
    u32 prev_offset;   // Previous offset for temporary allocations
} Arena;

// Default alignment (8 bytes for 64-bit compatibility)
#define ARENA_DEFAULT_ALIGNMENT 8

// Helper macro to align size up to the next boundary
#define ARENA_ALIGN_UP(size, alignment) \
    (((size) + (alignment) - 1) & ~((alignment) - 1))

// Create arena with provided memory
static inline Arena arena_init(void* memory, u32 size) {
    Arena arena = { 0 };
    arena.memory = (u8*)memory;
    arena.size = size;
    arena.offset = 0;
    arena.prev_offset = 0;
    return arena;
}

// Create an arena by allocating memory from the heap
static inline Arena arena_create(u32 size) {
    void* memory = malloc(size);
    if (!memory) {
        Arena empty = { 0 };
        return empty;
    }
    return arena_init(memory, size);
}

// Destroy an arena (free the memory if it was heap-allocated)
static inline void arena_destroy(Arena* arena) {
    if (arena && arena->memory) {
        free(arena->memory);
        memset(arena, 0, sizeof(Arena));
    }
}

// Reset the arena (mark all memory as available)
static inline void arena_reset(Arena* arena) {
    if (arena) {
        arena->offset = 0;
        arena->prev_offset = 0;
    }
}

// Get remaining bytes in the arena
static inline u32 arena_remaining(const Arena* arena) {
    if (!arena || arena->offset > arena->size) {
        return 0;
    }
    return arena->size - arena->offset;
}

// Get used bytes in the arena
static inline u32 arena_used(const Arena* arena) {
    return arena ? arena->offset : 0;
}

// Check if arena is valid
static inline bool arena_is_valid(const Arena* arena) {
    return arena && arena->memory && arena->size > 0;
}

// Allocate aligned memory from the arena
static inline void* arena_alloc_aligned(Arena* arena, u32 size, u32 alignment) {
    if (!arena_is_valid(arena) || size == 0) {
        return nullptr;
    }

    // Align the current offset
    const u32 aligned_offset = ARENA_ALIGN_UP(arena->offset, alignment);

    // Check if we have enough space
    if (aligned_offset + size > arena->size) {
        return nullptr; // Out of memory
    }

    // Update offset and return pointer
    arena->offset = aligned_offset + size;
    return arena->memory + aligned_offset;
}

// Allocate memory with default alignment
static inline void* arena_alloc(Arena* arena, u32 size) {
    return arena_alloc_aligned(arena, size, ARENA_DEFAULT_ALIGNMENT);
}

// Allocate and zero-initialize memory
static inline void* arena_calloc(Arena* arena, u32 count, u32 size) {
    u32 total_size = count * size;
    void* ptr = arena_alloc(arena, total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

// Allocate memory for a specific type
#define arena_alloc_type(arena, type) \
    (type*)arena_alloc_aligned(arena, sizeof(type), alignof(type))

// Allocate array of a specific type
#define arena_alloc_array(arena, type, count) \
    (type*)arena_alloc_aligned(arena, sizeof(type) * (count), alignof(type))

// Duplicate a string in the arena
static inline char* arena_strdup(Arena* arena, const char* str) {
    if (!str) return nullptr;

    u32 len = (u32) strlen(str) + 1; // Include null terminator
    char* copy = (char*)arena_alloc(arena, len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

// Duplicate a string with length limit
static inline char* arena_strndup(Arena* arena, const char* str, u32 max_len) {
    if (!str) return nullptr;

    u32 len = 0;
    while (len < max_len && str[len] != '\0') {
        len++;
    }

    char* copy = (char*)arena_alloc(arena, len + 1);
    if (copy) {
        memcpy(copy, str, len);
        copy[len] = '\0';
    }
    return copy;
}

// Format a string in the arena (like sprintf)
static inline char* arena_sprintf(Arena* arena, const char* format, ...) {
    va_list args;

    // First pass: determine required size
    va_start(args, format);
    i32 size = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    if (size < 0) return nullptr;

    // Allocate memory
    char* buffer = (char*)arena_alloc(arena, size + 1);
    if (!buffer) return nullptr;

    // Second pass: format the string
    va_start(args, format);
    vsnprintf(buffer, size + 1, format, args);
    va_end(args);

    return buffer;
}

// Save current arena state for temporary allocations
static inline u32 arena_save(Arena* arena) {
    if (!arena) return 0;
    arena->prev_offset = arena->offset;
    return arena->offset;
}

// Restore arena to saved state (free all temporary allocations)
static inline void arena_restore(Arena* arena, u32 saved_offset) {
    if (arena && saved_offset <= arena->size) {
        arena->offset = saved_offset;
    }
}

// Convenience macro for temporary allocations
#define ARENA_TEMP_SCOPE(arena) \
    for (u32 _temp_save = arena_save(arena), _temp_done = 0; \
         !_temp_done; \
         arena_restore(arena, _temp_save), _temp_done = 1)

typedef struct ArenaStats {
    u32 total_size;
    u32 used_bytes;
    u32 free_bytes;
    f64 utilization; // Used / Total as percentage
} ArenaStats;

static inline ArenaStats arena_get_stats(const Arena* arena) {
    ArenaStats stats = { 0 };
    if (arena_is_valid(arena)) {
        stats.total_size = arena->size;
        stats.used_bytes = arena->offset;
        stats.free_bytes = arena->size - arena->offset;
        stats.utilization = (f64)arena->offset / arena->size * 100.0;
    }
    return stats;
}

static inline void arena_print_stats(const Arena* arena, const char* name) {
    ArenaStats stats = arena_get_stats(arena);
    printf("Arena '%s':\n", name ? name : "Unknown");
    printf("  Total Size: %u bytes (%.2f KB)\n",
        stats.total_size, Kilobyte(stats.total_size));
    printf("  Used:       %u bytes (%.2f KB)\n",
        stats.used_bytes, Kilobyte(stats.used_bytes));
    printf("  Free:       %u bytes (%.2f KB)\n",
        stats.free_bytes, Kilobyte(stats.free_bytes));
    printf("  Utilization: %.1f%%\n", stats.utilization);
}

// Example structure
typedef struct Person {
    char* name;
    i32 age;
    float height;
} Person;

static void example_basic_usage(void) {
    printf("=== Basic Arena Usage ===\n");

    // Create a 1KB arena
    Arena arena = arena_create(1024);
    if (!arena_is_valid(&arena)) {
        printf("Failed to create arena\n");
        return;
    }

    // Allocate some integers
    i32* numbers = arena_alloc_array(&arena, i32, 10);
    for (i32 i = 0; i < 10; i++) {
        numbers[i] = i * i;
    }

    // Allocate a person struct
    Person* person = arena_alloc_type(&arena, Person);
    person->name = arena_strdup(&arena, "John Doe");
    person->age = 30;
    person->height = 5.9f;

    // Allocate some strings
    char* greeting = arena_sprintf(&arena, "Hello, %s! You are %d years old.",
        person->name, person->age);

    printf("Numbers: ");
    for (i32 i = 0; i < 10; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    printf("Person: %s, Age: %d, Height: %.1f\n",
        person->name, person->age, person->height);
    printf("Greeting: %s\n", greeting);

    arena_print_stats(&arena, "Basic Example");

    arena_destroy(&arena);
}

static void example_temporary_allocations(void) {
    printf("\n=== Temporary Allocations ===\n");

    Arena arena = arena_create(1024);

    // Allocate some permanent data
    i32* permanent = arena_alloc_array(&arena, i32, 5);
    for (i32 i = 0; i < 5; i++) {
        permanent[i] = i + 1;
    }

    printf("Permanent data allocated\n");
    arena_print_stats(&arena, "After permanent allocation");

    // Temporary scope
    ARENA_TEMP_SCOPE(&arena) {
        // These allocations will be freed when scope ends
        char* temp_str = arena_strdup(&arena, "This is temporary");
        i32* temp_array = arena_alloc_array(&arena, i32, 100);

        // Fill temp array
        for (i32 i = 0; i < 100; i++) {
            temp_array[i] = i;
        }

        printf("Temporary string: %s\n", temp_str);
        printf("Temporary data allocated\n");
        arena_print_stats(&arena, "During temporary scope");
    }

    // Temporary allocations are now freed
    printf("Back to permanent data only\n");
    arena_print_stats(&arena, "After temporary scope");

    // Permanent data is still valid
    printf("Permanent data: ");
    for (i32 i = 0; i < 5; i++) {
        printf("%d ", permanent[i]);
    }
    printf("\n");

    arena_destroy(&arena);
}

static void example_stack_arena(void) {
    printf("\n=== Stack-based Arena ===\n");

    // Create arena using stack memory
    u8 stack_memory[512];
    Arena arena = arena_init(stack_memory, sizeof(stack_memory));

    // Use it normally
    char* message = arena_sprintf(&arena, "This arena uses stack memory!");
    i32* data = arena_alloc_array(&arena, i32, 20);

    for (i32 i = 0; i < 20; i++) {
        data[i] = i * 2;
    }

    printf("Message: %s\n", message);
    printf("Data: ");
    for (i32 i = 0; i < 10; i++) { // Print first 10
        printf("%d ", data[i]);
    }
    printf("...\n");

    arena_print_stats(&arena, "Stack Arena");

    // No need to call arena_destroy() for stack-based arenas
}
