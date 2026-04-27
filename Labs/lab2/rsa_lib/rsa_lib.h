#pragma once

#ifdef RSA_EXPORTS
#define RSA_API __declspec(dllexport)
#else
#define RSA_API __declspec(dllimport)
#endif

#include <cstdint>

struct RSAKey {
    uint64_t e; // відкритий ключ
    uint64_t d; // закритий ключ
    uint64_t n; // модуль
};

extern "C" {
    RSA_API bool isPrime(uint64_t n);
    RSA_API uint64_t gcd(uint64_t a, uint64_t b);
    RSA_API uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod);
    RSA_API uint64_t modInverse(uint64_t a, uint64_t m);
    RSA_API RSAKey generateKeys(uint64_t p, uint64_t q);
    RSA_API uint64_t encrypt(uint64_t M, uint64_t e, uint64_t n);
    RSA_API uint64_t decrypt(uint64_t C, uint64_t d, uint64_t n);
}
