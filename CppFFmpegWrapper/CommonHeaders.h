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
#include <string>

#include "TypesDefine.h"

constexpr u32 frame_buffer_count = 3;

#include "CallbackType.h"
#include "PacketDefine.h"
