#pragma once
#include <cstdint>

// Експорт через rsa_lib_dyn.def, тому без __declspec(dllexport)
struct RSAKey {
    uint64_t e;
    uint64_t d;
    uint64_t n;
};

extern "C" {
    bool isPrime(uint64_t n);
    uint64_t gcd(uint64_t a, uint64_t b);
    uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod);
    uint64_t modInverse(uint64_t a, uint64_t m);
    RSAKey generateKeys(uint64_t p, uint64_t q);
    uint64_t encrypt(uint64_t M, uint64_t e, uint64_t n);
    uint64_t decrypt(uint64_t C, uint64_t d, uint64_t n);
}
