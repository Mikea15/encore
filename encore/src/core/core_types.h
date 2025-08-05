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

#define SecondsToMinutes(s) s / 60
#define MinutesToHours(m) m / 60

#define MsToSeconds(ms) ms / 1000
#define NsToMilliseconds(ns) ns / 1000

#define SecToMs(s) (f32)s * 1000
#define MsToNs(s) (f32)s * 1000
#define MsToSec(s) (f32)s / 1000

#define BytesToKB(b) b / 1024
#define BytesToMB(b) BytesToKB(b) / 1024
#define BytesToGB(b) BytesToMB(b) / 1024

#define KB(n) n * 1024
#define MB(n) KB(n) * 1024
#define GB(n) MB(n) * 1024

#define BIT(b) (1 << b)
