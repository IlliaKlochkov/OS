#include <iostream>
#include <cstdint>
#include "rsa_lib.h"

int main() {
    uint64_t p0 = 999907, q0 = 999917;
    uint64_t p1 = 999931, q1 = 999953;

    RSAKey key0 = generateKeys(p0, q0);
    RSAKey key1 = generateKeys(p1, q1);

    std::cout << "Key 0: e=" << key0.e << " d=" << key0.d << " n=" << key0.n << "\n";
    std::cout << "Key 1: e=" << key1.e << " d=" << key1.d << " n=" << key1.n << "\n\n";

    uint64_t testData[] = { 12345, 9999, 1, 42, 777777 };
    int count = sizeof(testData) / sizeof(testData[0]);
    bool allOk = true;

    for (int i = 0; i < count; i++) {
        uint64_t t = testData[i];
        if (t >= key0.n || t >= key1.n) {
            std::cout << "t[" << i << "]=" << t << " skipped (too large)\n";
            continue;
        }

        uint64_t e1t = encrypt(t, key1.e, key1.n);
        uint64_t d1e1t = decrypt(e1t, key1.d, key1.n);
        if (d1e1t != t) {
            std::cout << "ERROR at step 1, i=" << i << "\n";
            allOk = false; continue;
        }

        uint64_t e0d1e1t = encrypt(d1e1t, key0.e, key0.n);
        uint64_t d0e0d1e1t = decrypt(e0d1e1t, key0.d, key0.n);
        if (d0e0d1e1t != t) {
            std::cout << "ERROR at step 2, i=" << i << "\n";
            allOk = false; continue;
        }

        std::cout << "t[" << i << "]=" << t
            << " -> e1=" << e1t
            << " -> d1=" << d1e1t
            << " -> e0=" << e0d1e1t
            << " -> d0=" << d0e0d1e1t
            << " OK\n";
    }

    std::cout << "\n" << (allOk ? "All tests PASSED." : "Some tests FAILED.") << "\n";
    return 0;
}