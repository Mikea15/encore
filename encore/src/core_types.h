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

#define SecToMs(s) (f32)s * 1000
#define MsToSec(s) (f32)s / 1000

#define Kilobyte(b) (f32) b * 1024
#define Megabyte(b) Kilobyte(b) * 1024
#define Gigabyte(b) Megabyte(b) * 1024

#define BIT(b) (1 << b)
