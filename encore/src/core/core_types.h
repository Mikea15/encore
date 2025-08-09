#pragma once

#include <stdint.h>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using b8 = bool;

using f32 = float;
using f64 = double;

// From NanoSeconds
#define NS_TO_US(ns)    ((ns) / 1'000ULL)
#define NS_TO_MS(ns)    ((ns) / 1'000'000ULL)
#define NS_TO_SEC(ns)   ((ns) / 1'000'000'000ULL)

// From Microseconds
#define US_TO_MS(us)    ((us) / 1'000ULL)
#define US_TO_SEC(us)   ((us) / 1'000'000ULL)

// From Milliseconds
#define MS_TO_SEC(ms)   ((ms) / 1'000ULL)

// Time
#define HOUR_TO_DAY(h)  ((h) / 24ULL)
#define MIN_TO_HOUR(m)  ((m) / 60ULL)
#define SEC_TO_MIN(s)   ((s) / 60ULL)

// To Nanoseconds
#define SEC_TO_NS(s)    ((s) * 1'000'000'000ULL)
#define MS_TO_NS(ms)    ((ms) * 1'000'000ULL)
#define US_TO_NS(us)    ((us) * 1'000ULL)

// To MicroSeconds
#define SEC_TO_US(s)    ((s) * 1'000'000ULL)
#define MS_TO_US(ms)    ((ms) * 1'000ULL)

// To Milliseconds
#define SEC_TO_MS(s)    ((s) * 1'000ULL)

// Time
#define DAY_TO_HOUR(d)  ((d) * 24ULL)
#define HOUR_TO_MIN(h)  ((h) * 60ULL)
#define MIN_TO_SEC(m)   ((m) * 60ULL)

// Input Seconds
#define NANOSECONDS(n)  (n)
#define MICROSECONDS(n) SEC_TO_US(n)
#define MILLISECONDS(n) SEC_TO_MS(n)
#define SECONDS(n)      SEC_TO_NS(n)
#define MINUTES(n)      SEC_TO_NS(MIN_TO_SEC(n))
#define HOURS(n)        SEC_TO_NS(HOUR_TO_MIN(MIN_TO_SEC(n)))

// From Bytes
#define BYTES_TO_TB(b)  ((b) / (1024ULL * 1024ULL * 1024ULL * 1024ULL))
#define BYTES_TO_GB(b)  ((b) / (1024ULL * 1024ULL * 1024ULL))
#define BYTES_TO_MB(b)  ((b) / (1024ULL * 1024ULL))
#define BYTES_TO_KB(b)  ((b) / 1024ULL)

// From KiloBytes
#define KB_TO_TB(kb)    ((kb) / (1024ULL * 1024ULL * 1024ULL))
#define KB_TO_GB(kb)    ((kb) / (1024ULL * 1024ULL))
#define KB_TO_MB(kb)    ((kb) / 1024ULL)

// From Megabytes
#define MB_TO_TB(mb)    ((mb) / (1024ULL * 1024ULL))
#define MB_TO_GB(mb)    ((mb) / 1024ULL)

// From Gigabytes
#define GB_TO_TB(gb)    ((gb) / 1024ULL)

// To Bytes
#define TB_TO_BYTES(tb) ((tb) * 1024ULL * 1024ULL * 1024ULL * 1024ULL)
#define GB_TO_BYTES(gb) ((gb) * 1024ULL * 1024ULL * 1024ULL)
#define MB_TO_BYTES(mb) ((mb) * 1024ULL * 1024ULL)
#define KB_TO_BYTES(kb) ((kb) * 1024ULL)

// To Kilobytes
#define TB_TO_KB(tb)    ((tb) * 1024ULL * 1024ULL * 1024ULL)
#define GB_TO_KB(gb)    ((gb) * 1024ULL * 1024ULL)
#define MB_TO_KB(mb)    ((mb) * 1024ULL)

// To Megabytes
#define TB_TO_MB(tb)    ((tb) * 1024ULL * 1024ULL)
#define GB_TO_MB(gb)    ((gb) * 1024ULL)

// To Gigabytes
#define TB_TO_GB(tb)    ((tb) * 1024ULL)

// Return Bytes
#define BYTES(n)        (n)
#define KILOBYTES(n)    KB_TO_BYTES(n)
#define MEGABYTES(n)    MB_TO_BYTES(n)
#define GIGABYTES(n)    GB_TO_BYTES(n)
#define TERABYTES(n)    TB_TO_BYTES(n)

// Shorter aliases for convenience constructors
#define KB(n)           KILOBYTES(n)
#define MB(n)           MEGABYTES(n)
#define GB(n)           GIGABYTES(n)
#define TB(n)           TERABYTES(n)

// From Bytes
#define BYTES_TO_TIB(b) ((b) / (1000ULL * 1000ULL * 1000ULL * 1000ULL))
#define BYTES_TO_GIB(b) ((b) / (1000ULL * 1000ULL * 1000ULL))
#define BYTES_TO_MIB(b) ((b) / (1000ULL * 1000ULL))
#define BYTES_TO_KIB(b) ((b) / 1000ULL)

// To Bytes
#define TIB_TO_BYTES(t) ((t) * 1000ULL * 1000ULL * 1000ULL * 1000ULL)
#define GIB_TO_BYTES(g) ((g) * 1000ULL * 1000ULL * 1000ULL)
#define MIB_TO_BYTES(m) ((m) * 1000ULL * 1000ULL)
#define KIB_TO_BYTES(k) ((k) * 1000ULL)

// Returns Bytes
#define KIBIBYTES(n)    KIB_TO_BYTES(n)
#define MEBIBYTES(n)    MIB_TO_BYTES(n)
#define GIBIBYTES(n)    GIB_TO_BYTES(n)
#define TEBIBYTES(n)    TIB_TO_BYTES(n)

#define BIT(b) (1 << (b))

#define NO_COPY(Type)						\
	Type(const Type&) = delete;				\
	Type& operator=(const Type&) = delete

#define NO_MOVE(Type)						\
	Type(Type&&) = delete;					\
	Type& operator=(Type&&) = delete
