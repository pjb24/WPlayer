#pragma once

#include <cstdint>
#include <assert.h>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <cstring>
#include <thread>
#include <chrono>

// unsigned integers
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

// signed integers
using s64 = int64_t;
using s32 = int32_t;
using s16 = int16_t;
using s8 = int8_t;

constexpr u64 u64_invalid_id = 0xffff'ffff'ffff'ffff;
constexpr u32 u32_invalid_id = 0xffff'ffff;
constexpr u16 u16_invalid_id = 0xffff;
constexpr u8 u8_invalid_id = 0xff;

using f32 = float;

constexpr u32 frame_buffer_count = 3;

#include "CallbackType.h"
#include "PacketDefine.h"
