#ifndef __HAKKA_JSON_ARRAY_H__
#define __HAKKA_JSON_ARRAY_H__
#pragma once
#include <common.h>

// Creation and Destruction
extern_c HakkaJsonResultEnum CreateHakkaArray(HakkaHandle *handle);
extern_c HakkaJsonResultEnum LoadsHakkaArray(const uint8_t *json_str, uint32_t json_length, HakkaHandle *handle, uint32_t max_depth);
extern_c HakkaJsonResultEnum DumpHakkaArray(HakkaHandle array, uint32_t max_depth, uint8_t *buffer, uint64_t *buffer_size);

// Array Manipulation
extern_c HakkaJsonResultEnum GetHakkaArrayObject(HakkaHandle array, uint32_t index, HakkaHandle *value); // value is generic handle
extern_c HakkaJsonResultEnum SetHakkaArray(HakkaHandle array, uint32_t index, HakkaHandle value);        // value is generic handle
extern_c HakkaJsonResultEnum GetHakkaArraySlice(HakkaHandle array, int64_t start, int64_t end, int64_t step, HakkaHandle *result);
extern_c HakkaJsonResultEnum SetHakkaArraySlice(HakkaHandle dst, int64_t start, int64_t end, int64_t step, HakkaHandle src);
extern_c HakkaJsonResultEnum RemoveHakkaArrayIndex(HakkaHandle array, uint32_t index);
extern_c HakkaJsonResultEnum ClearHakkaArray(HakkaHandle array);
extern_c HakkaJsonResultEnum InsertHakkaArray(HakkaHandle array, uint32_t index, HakkaHandle value);

// Additional methods to support C API operations with Python list-like behavior
extern_c HakkaJsonResultEnum MultiplyHakkaArray(HakkaHandle array, int64_t times);
extern_c HakkaJsonResultEnum GetHakkaArraySize(HakkaHandle array, uint32_t *size);
extern_c HakkaJsonResultEnum CountHakkaArray(HakkaHandle array, HakkaHandle value, uint32_t *count);
extern_c HakkaJsonResultEnum ExtendHakkaArrayArray(HakkaHandle array, HakkaHandle value);
extern_c HakkaJsonResultEnum FindFirstHakkaArray(HakkaHandle array, HakkaHandle other, uint32_t start, uint32_t stop, uint32_t *index); // used for index() method
extern_c HakkaJsonResultEnum PushBackHakkaArray(HakkaHandle array, HakkaHandle value);
extern_c HakkaJsonResultEnum PopHakkaArray(HakkaHandle array, uint32_t index, HakkaHandle *value);
extern_c HakkaJsonResultEnum RemoveValueHakkaArray(HakkaHandle array, HakkaHandle value); // remove first occurrence of value
extern_c HakkaJsonResultEnum ReverseHakkaArray(HakkaHandle array);

// Iters
extern_c HakkaJsonResultEnum CreateHakkaArrayIterBegin(HakkaHandle array, HakkaArrayIter *iter);
extern_c HakkaJsonResultEnum CreateHakkaArrayIterRBegin(HakkaHandle array, HakkaArrayIter *iter);
extern_c HakkaJsonResultEnum MoveHakkaArrayIterNext(HakkaArrayIter iter);
extern_c HakkaJsonResultEnum MoveHakkaArrayIterPrev(HakkaArrayIter iter);
extern_c HakkaJsonResultEnum GetHakkaArrayIterDeref(HakkaArrayIter iter, HakkaHandle *value); // value is generic handle
extern_c HakkaJsonResultEnum HakkaArrayIterRelease(HakkaArrayIter *iter);

#endif // __HAKKA_JSON_ARRAY_H__