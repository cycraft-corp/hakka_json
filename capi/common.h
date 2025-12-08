#ifndef __HAKKA_COMMON_H__
#define __HAKKA_COMMON_H__
#pragma once

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif

#include <hakka_json_enum.h>
#include <hakka_json_export.h>

#define extern_c extern "C" HAKKA_JSON_EXPORT
#define C_BOOL uint8_t

#ifdef __cplusplus
using HakkaHandle = uint64_t;
using HakkaStringIter = uint64_t;
using HakkaArrayIter = uint64_t;
using HakkaObjectIter = uint64_t;
#else
typedef uint64_t HakkaHandle;
typedef uint64_t HakkaStringIter;
typedef uint64_t HakkaArrayIter;
typedef uint64_t HakkaObjectIter;
#endif

#endif // __HAKKA_COMMON_H__
