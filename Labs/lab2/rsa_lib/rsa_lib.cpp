#include "rsa_lib.h"
#include <cstdint>

bool isPrime(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (uint64_t i = 3; i <= n / i; i += 2)
        if (n % i == 0) return false;
    return true;
}

uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) { uint64_t t = b; b = a % b; a = t; }
    return a;
}

static uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m) {
    a %= m;
    uint64_t result = 0;
    while (b) {
        if (b & 1) {
            if (result >= m - a) result = result - (m - a);
            else result += a;
        }
        b >>= 1;
        if (b) {
            if (a >= m - a) a = a - (m - a);
            else a += a;
        }
    }
    return result;
}

uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = mulmod(result, base, mod);
        base = mulmod(base, base, mod);
        exp >>= 1;
    }
    return result;
}

uint64_t modInverse(uint64_t a, uint64_t m) {
    int64_t t = 0, newt = 1;
    int64_t r = (int64_t)m, newr = (int64_t)a;
    while (newr != 0) {
        int64_t q = r / newr;
        int64_t tmp = t - q * newt; t = newt; newt = tmp;
        tmp = r - q * newr; r = newr; newr = tmp;
    }
    if (r > 1) return 0;
    if (t < 0) t += (int64_t)m;
    return (uint64_t)t;
}

RSAKey generateKeys(uint64_t p, uint64_t q) {
    RSAKey key;
    key.n = p * q;
    uint64_t phi = (p - 1) * (q - 1);
    key.e = 65537;
    if (gcd(key.e, phi) != 1) {
        key.e = 3;
        while (gcd(key.e, phi) != 1) key.e += 2;
    }
    key.d = modInverse(key.e, phi);
    return key;
}

uint64_t encrypt(uint64_t M, uint64_t e, uint64_t n) {
    return modPow(M, e, n);
}

uint64_t decrypt(uint64_t C, uint64_t d, uint64_t n) {
    return modPow(C, d, n);
}
