#ifndef __HAKKA_JSON_ENUM_H__
#define __HAKKA_JSON_ENUM_H__
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
        HAKKA_JSON_SUCCESS = 0,
        HAKKA_JSON_PARSE_ERROR,
        HAKKA_JSON_TYPE_ERROR,
        HAKKA_JSON_NOT_ENOUGH_MEMORY,
        HAKKA_JSON_KEY_NOT_FOUND,
        HAKKA_JSON_INDEX_OUT_OF_BOUNDS,
        HAKKA_JSON_INVALID_ARGUMENT,
        HAKKA_JSON_OVERFLOW,
        HAKKA_JSON_RECURSION_DEPTH_EXCEEDED,
        HAKKA_JSON_ITERATOR_END,
        HAKKA_JSON_INTERNAL_ERROR = -1,
    } HakkaJsonResultEnum;

    typedef enum
    {
        HAKKA_JSON_NULL = 0,
        HAKKA_JSON_STRING,
        HAKKA_JSON_INT,
        HAKKA_JSON_FLOAT,
        HAKKA_JSON_BOOL,
        HAKKA_JSON_OBJECT,
        HAKKA_JSON_ARRAY,
        HAKKA_JSON_INVALID = -1,
    } HakkaJsonType;

#ifdef __cplusplus
}
#endif

#endif // __HAKKA_JSON_ENUM_H__