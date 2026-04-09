#include "pch.h" 
#include "mymath.h"
#include <limits.h>

// реалізація з перевіркою на критичні значення (переповнення типу long long)
MYMATH_API bool MyAdd(long long a, long long b, long long* res) {
    if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b)) {
        return false;
    }
    *res = a + b;
    return true;
}

MYMATH_API bool MySub(long long a, long long b, long long* res) {
    if ((b > 0 && a < LLONG_MIN + b) || (b < 0 && a > LLONG_MAX + b)) {
        return false;
    }
    *res = a - b;
    return true;
}

MYMATH_API bool MyMul(long long a, long long b, long long* res) {
    if (a == 0 || b == 0) {
        *res = 0;
        return true;
    }

    // перевірка специфічного випадку для мінімального значення
    if (a == -1 && b == LLONG_MIN) return false;
    if (b == -1 && a == LLONG_MIN) return false;

    // перевірка переповнення через ділення ліміту на множник
    if (a > 0 && b > 0 && a > LLONG_MAX / b) return false;
    if (a > 0 && b < 0 && b < LLONG_MIN / a) return false;
    if (a < 0 && b > 0 && a < LLONG_MIN / b) return false;
    if (a < 0 && b < 0 && a < LLONG_MAX / b) return false;

    *res = a * b;
    return true;
}

MYMATH_API bool MyDiv(long long a, long long b, long long* res) {
    if (b == 0) return false; // захист від ділення на нуль
    if (a == LLONG_MIN && b == -1) return false;

    *res = a / b;
    return true;
}