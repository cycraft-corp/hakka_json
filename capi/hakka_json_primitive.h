#ifndef __HAKKA_JSON_PRIMITIVE_H__
#define __HAKKA_JSON_PRIMITIVE_H__
#pragma once

#include <common.h>

extern_c void HakkaRelease(HakkaHandle *handle);

// Base
extern_c HakkaJsonResultEnum HakkaDump(HakkaHandle handle, uint32_t max_depth, uint8_t *buffer, uint64_t *buffer_size);
extern_c HakkaJsonResultEnum HakkaToBytes(HakkaHandle handle, uint8_t *buffer, uint32_t *buffer_size);
extern_c HakkaJsonResultEnum HakkaIsValid(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum HakkaType(HakkaHandle handle, HakkaJsonType *type);
extern_c HakkaJsonResultEnum HakkaCompare(HakkaHandle handle, HakkaHandle other, int32_t *result);
extern_c HakkaJsonResultEnum HakkaHash(HakkaHandle handle, uint64_t *hash);
extern_c HakkaJsonResultEnum HakkaDumpSize(HakkaHandle handle, uint64_t *capacity);
extern_c HakkaJsonResultEnum HakkaReclaim(HakkaHandle handle); // propagate from python to reclaim memory, they use release to release handle

// Primitive int, float, null, invalid
extern_c HakkaJsonResultEnum CreateHakkaInt(HakkaHandle *handle, int64_t value);
extern_c HakkaJsonResultEnum CreateHakkaFloat(HakkaHandle *handle, double value);
extern_c HakkaJsonResultEnum CreateHakkaNull(HakkaHandle *handle);
extern_c HakkaJsonResultEnum CreateHakkaBool(HakkaHandle *handle, C_BOOL value);
extern_c HakkaJsonResultEnum CreateHakkaInvalid(HakkaHandle *handle);
extern_c HakkaJsonResultEnum GetHakkaInt(HakkaHandle handle, int64_t *value);
extern_c HakkaJsonResultEnum GetHakkaFloat(HakkaHandle handle, double *value);
extern_c HakkaJsonResultEnum GetHakkaBool(HakkaHandle handle, C_BOOL *value);

// string
extern_c HakkaJsonResultEnum CreateHakkaString(HakkaHandle *handle, const uint8_t *value, uint32_t length);
extern_c HakkaJsonResultEnum GetHakkaString(HakkaHandle handle, uint8_t *buffer, uint32_t *buffer_size);
extern_c HakkaJsonResultEnum GetHakkaStringLength(HakkaHandle handle, uint32_t *length);
extern_c HakkaJsonResultEnum GetHakkaStringCapitalize(HakkaHandle handle, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringCasefold(HakkaHandle handle, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringCount(HakkaHandle handle, const uint8_t *substring, uint32_t substring_length, int64_t *count);
extern_c HakkaJsonResultEnum GetHakkaStringEndswith(HakkaHandle handle, const uint8_t *suffix, uint32_t suffix_length, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringFind(HakkaHandle handle, const uint8_t *substring, uint32_t substring_length, int64_t *position);
extern_c HakkaJsonResultEnum GetHakkaStringConcatenate(HakkaHandle handle, const uint8_t *other, uint32_t other_length, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringMultiply(HakkaHandle handle, int64_t times, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringSlice(HakkaHandle handle, int64_t start, int64_t end, int64_t step, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringLower(HakkaHandle handle, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringRemoveprefix(HakkaHandle handle, const uint8_t *prefix, uint32_t prefix_length, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringRemovesuffix(HakkaHandle handle, const uint8_t *suffix, uint32_t suffix_length, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringReplace(HakkaHandle handle, const uint8_t *old_substr, uint32_t old_substr_length, const uint8_t *new_substr, uint32_t new_substr_length, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringRfind(HakkaHandle handle, const uint8_t *substring, uint32_t substring_length, int64_t *position);
extern_c HakkaJsonResultEnum GetHakkaStringRsplit(HakkaHandle handle, const uint8_t *separator, uint32_t separator_length, int64_t maxsplit, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringSplit(HakkaHandle handle, const uint8_t *separator, uint32_t separator_length, int64_t maxsplit, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringSplitlines(HakkaHandle handle, C_BOOL keepends, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringStartswith(HakkaHandle handle, const uint8_t *prefix, uint32_t prefix_length, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringUpper(HakkaHandle handle, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringSwapcase(HakkaHandle handle, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringTitle(HakkaHandle handle, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringZfill(HakkaHandle handle, int64_t width, HakkaHandle *result);
extern_c HakkaJsonResultEnum GetHakkaStringUTF8Length(HakkaHandle handle, uint64_t *length);

// String testing functions
extern_c HakkaJsonResultEnum GetHakkaStringIsalnum(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsalpha(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsascii(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsdecimal(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsdigit(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsidentifier(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIslower(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsnumeric(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsprintable(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsspace(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIstitle(HakkaHandle handle, C_BOOL *result);
extern_c HakkaJsonResultEnum GetHakkaStringIsupper(HakkaHandle handle, C_BOOL *result);

// string iterator
extern_c HakkaJsonResultEnum CreateHakkaStringBegin(HakkaHandle str_handle, HakkaStringIter *iter);
extern_c HakkaJsonResultEnum MoveHakkaStringNext(HakkaStringIter iter);
extern_c HakkaJsonResultEnum GetHakkaStringDeref(HakkaStringIter iter, uint32_t *utf32);
extern_c HakkaJsonResultEnum HakkaStringIterRelease(HakkaStringIter *iter);

#endif
