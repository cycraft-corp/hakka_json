#ifndef HAKKA_JSON_EXPORT_H
#define HAKKA_JSON_EXPORT_H

// Symbol visibility macros for hakka_json library
// This header defines macros for controlling symbol visibility in shared libraries

#if defined(_WIN32) || defined(__CYGWIN__)
    // Windows DLL export/import
    #ifdef HAKKA_JSON_BUILDING_DLL
        #define HAKKA_JSON_EXPORT __declspec(dllexport)
    #else
        #define HAKKA_JSON_EXPORT __declspec(dllimport)
    #endif
    #define HAKKA_JSON_LOCAL
#else
    // GCC/Clang visibility attributes
    #if defined(__GNUC__) && __GNUC__ >= 4
        #define HAKKA_JSON_EXPORT __attribute__((visibility("default")))
        #define HAKKA_JSON_LOCAL  __attribute__((visibility("hidden")))
    #else
        #define HAKKA_JSON_EXPORT
        #define HAKKA_JSON_LOCAL
    #endif
#endif

// C API functions are always exported
#ifdef __cplusplus
extern "C" {
#endif

// All C API functions should be declared with HAKKA_JSON_EXPORT
// Example: HAKKA_JSON_EXPORT void hakka_json_function(void);

#ifdef __cplusplus
}
#endif

#endif // HAKKA_JSON_EXPORT_H
