#pragma once

// макрос для керування експортом/імпортом функцій
#ifdef MYMATH_EXPORTS
#define MYMATH_API extern "C" __declspec(dllexport)
#else
#define MYMATH_API extern "C" __declspec(dllimport)
#endif

MYMATH_API bool MyAdd(long long a, long long b, long long* res);
MYMATH_API bool MySub(long long a, long long b, long long* res);
MYMATH_API bool MyMul(long long a, long long b, long long* res);
MYMATH_API bool MyDiv(long long a, long long b, long long* res);