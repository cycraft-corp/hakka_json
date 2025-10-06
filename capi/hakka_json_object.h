#ifndef __HAKKA_JSON_OBJECT_H__
#define __HAKKA_JSON_OBJECT_H__
#pragma once

#include <common.h>

// Creation and Destruction
extern_c HakkaJsonResultEnum CreateHakkaObject(HakkaHandle *handle);
extern_c HakkaJsonResultEnum LoadsHakkaObject(const uint8_t *json_str, uint32_t json_length, HakkaHandle *handle, uint32_t max_depth);
extern_c HakkaJsonResultEnum DumpHakkaObject(HakkaHandle object, uint32_t max_depth, uint8_t *buffer, uint64_t *buffer_size);

// Object Manipulation
extern_c HakkaJsonResultEnum SetHakkaObjectInt(HakkaHandle object, const uint8_t *key, uint32_t key_length, int64_t value);
extern_c HakkaJsonResultEnum SetHakkaObjectFloat(HakkaHandle object, const uint8_t *key, uint32_t key_length, double value);
extern_c HakkaJsonResultEnum SetHakkaObjectString(HakkaHandle object, const uint8_t *key, uint32_t key_length, const uint8_t *value, uint32_t value_length);
extern_c HakkaJsonResultEnum SetHakkaObjectNull(HakkaHandle object, const uint8_t *key, uint32_t key_length);
extern_c HakkaJsonResultEnum GetHakkaObjectInt(HakkaHandle object, const uint8_t *key, uint32_t key_length, int64_t *value);
extern_c HakkaJsonResultEnum GetHakkaObjectFloat(HakkaHandle object, const uint8_t *key, uint32_t key_length, double *value);
extern_c HakkaJsonResultEnum GetHakkaObjectString(HakkaHandle object, const uint8_t *key, uint32_t key_length, uint8_t *buffer, uint32_t *buffer_size);
extern_c HakkaJsonResultEnum GetHakkaObjectNull(HakkaHandle object, const uint8_t *key, uint32_t key_length, C_BOOL *result);

extern_c HakkaJsonResultEnum GetHakkaObjectObject(HakkaHandle object, const uint8_t *key, uint32_t key_length, HakkaHandle *value);
extern_c HakkaJsonResultEnum SetHakkaObject(HakkaHandle object, const uint8_t *key, uint32_t key_length, HakkaHandle value);

// Additional methods to support C API operations with Python dict-like behavior
extern_c HakkaJsonResultEnum RemoveHakkaObjectKey(HakkaHandle object, const uint8_t *key, uint32_t key_length);
extern_c HakkaJsonResultEnum GetHakkaObjectSize(HakkaHandle object, uint32_t *size);
extern_c HakkaJsonResultEnum ContainsHakkaObjectKey(HakkaHandle object, const uint8_t *key, uint32_t key_length, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaObjectKeys(HakkaHandle object, HakkaHandle *keys);
extern_c HakkaJsonResultEnum GetHakkaObjectValues(HakkaHandle object, HakkaHandle *values);
extern_c HakkaJsonResultEnum CreateHakkaObjectFromKeys(HakkaHandle string_array_handle, HakkaHandle default_value, HakkaHandle *result);
extern_c HakkaJsonResultEnum PopHakkaObject(HakkaHandle object, const uint8_t *key, uint32_t key_length, HakkaHandle *value);
extern_c HakkaJsonResultEnum PopItemHakkaObject(HakkaHandle object, HakkaHandle *key, HakkaHandle *value);
extern_c HakkaJsonResultEnum ClearHakkaObject(HakkaHandle object);
extern_c HakkaJsonResultEnum UpdateHakkaObject(HakkaHandle object, HakkaHandle other);

// Iters
extern_c HakkaJsonResultEnum CreateHakkaObjectIterBegin(HakkaHandle object, HakkaObjectIter *iter);
extern_c HakkaJsonResultEnum MoveHakkaObjectIterNext(HakkaObjectIter iter);
extern_c HakkaJsonResultEnum GetHakkaObjectIterDeref(HakkaObjectIter iter, HakkaHandle *key, HakkaHandle *value); // key is string handle, another object handle
extern_c HakkaJsonResultEnum HakkaObjectIterRelease(HakkaObjectIter *iter);

#endif // __HAKKA_JSON_OBJECT_H__
