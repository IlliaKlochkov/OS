#include <windows.h>
#include <iostream>
#include <cstdint>

// Дублюємо опис ключа з rsa_lib_dyn — головна програма не лінкується з DLL,
// тому користуємося тільки сигнатурами функцій
struct RSAKey {
    uint64_t e;
    uint64_t d;
    uint64_t n;
};

// Типи вказівників на функції з DLL — для виклику через GetProcAddress
typedef RSAKey   (*pGenerateKeys)(uint64_t, uint64_t);
typedef uint64_t (*pEncrypt)(uint64_t, uint64_t, uint64_t);
typedef uint64_t (*pDecrypt)(uint64_t, uint64_t, uint64_t);

// Локальні ID мають збігатися з resource.h в обох ресурсних DLL
#define IDS_LASTNAME 101
#define IDS_FACULTY  102
#define IDS_GROUP    103
#define IDS_SUBJECT  104

static void PrintLocalizedString(HMODULE hRes, UINT id) {
    wchar_t wbuf[256] = {};
    int len = LoadStringW(hRes, id, wbuf, 256);
    if (len <= 0) { std::cout << "<missing string>\n"; return; }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written = 0;
    // Якщо stdout — справжня консоль, пишемо широкі символи напряму
    if (WriteConsoleW(hOut, wbuf, (DWORD)len, &written, nullptr)) {
        WriteConsoleW(hOut, L"\n", 1, &written, nullptr);
        return;
    }
    // Інакше (pipe/файл) — конвертуємо в UTF-8
    char ubuf[1024];
    int ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf, len, ubuf, sizeof(ubuf), nullptr, nullptr);
    if (ulen > 0) std::cout.write(ubuf, ulen);
    std::cout << "\n";
}

int wmain() {
    SetConsoleOutputCP(65001);
    std::wcout << L"=== Lab 3 - dynamic DLL loading ===\n\n";

    // 1. Динамічно завантажуємо DLL з RSA-функціями
    HMODULE hRsa = LoadLibraryW(L"rsa_lib_dyn.dll");
    if (!hRsa) {
        std::wcerr << L"LoadLibrary(rsa_lib_dyn.dll) failed: " << GetLastError() << L"\n";
        std::wcerr << L"(integrity check may have failed - try rebuilding)\n";
        return 1;
    }
    std::wcout << L"rsa_lib_dyn.dll loaded successfully\n\n";

    // 2. Отримуємо адреси функцій (внутрішні імена з .def)
    auto generateKeys = (pGenerateKeys)GetProcAddress(hRsa, "generateKeys");
    auto encrypt      = (pEncrypt)     GetProcAddress(hRsa, "encrypt");
    auto decrypt      = (pDecrypt)     GetProcAddress(hRsa, "decrypt");

    if (!generateKeys || !encrypt || !decrypt) {
        std::wcerr << L"GetProcAddress failed: " << GetLastError() << L"\n";
        FreeLibrary(hRsa);
        return 1;
    }

    // 3. RSA-тест аналогічний lab2: 2 ключі, шифр-дешифр на масиві даних
    uint64_t p0 = 999907, q0 = 999917;
    uint64_t p1 = 999931, q1 = 999953;

    RSAKey key0 = generateKeys(p0, q0);
    RSAKey key1 = generateKeys(p1, q1);

    std::wcout << L"Key 0: e=" << key0.e << L" d=" << key0.d << L" n=" << key0.n << L"\n";
    std::wcout << L"Key 1: e=" << key1.e << L" d=" << key1.d << L" n=" << key1.n << L"\n\n";

    uint64_t testData[] = { 12345, 9999, 1, 42, 777777 };
    int count = sizeof(testData) / sizeof(testData[0]);
    bool allOk = true;

    for (int i = 0; i < count; i++) {
        uint64_t t = testData[i];
        if (t >= key0.n || t >= key1.n) {
            std::wcout << L"t[" << i << L"]=" << t << L" skipped (too large)\n";
            continue;
        }

        uint64_t e1t = encrypt(t, key1.e, key1.n);
        uint64_t d1e1t = decrypt(e1t, key1.d, key1.n);
        if (d1e1t != t) { allOk = false; continue; }

        uint64_t e0d1 = encrypt(d1e1t, key0.e, key0.n);
        uint64_t d0e0 = decrypt(e0d1, key0.d, key0.n);
        if (d0e0 != t) { allOk = false; continue; }

        std::wcout << L"t[" << i << L"]=" << t
            << L" -> e1=" << e1t << L" -> d1=" << d1e1t
            << L" -> e0=" << e0d1 << L" -> d0=" << d0e0 << L" OK\n";
    }
    std::wcout << L"\n" << (allOk ? L"All RSA tests PASSED.\n" : L"Some RSA tests FAILED.\n");

    // 4. Запит мови та завантаження ресурсної DLL
    std::wcout << L"\nChoose language [1=Ukrainian, 2=English]: ";
    int choice = 1;
    std::cin >> choice;

    const wchar_t* resName = (choice == 2) ? L"resource_eng.dll" : L"resource_ukr.dll";
    // LOAD_LIBRARY_AS_DATAFILE — DllMain не викликається, потрібні лише ресурси
    HMODULE hRes = LoadLibraryExW(resName, nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (!hRes) {
        std::wcerr << L"Cannot load resource DLL: " << resName
            << L" (err=" << GetLastError() << L")\n";
        FreeLibrary(hRsa);
        return 1;
    }

    std::wcout << L"\n--- " << resName << L" ---\n";
    PrintLocalizedString(hRes, IDS_LASTNAME);
    PrintLocalizedString(hRes, IDS_FACULTY);
    PrintLocalizedString(hRes, IDS_GROUP);
    PrintLocalizedString(hRes, IDS_SUBJECT);

    FreeLibrary(hRes);
    FreeLibrary(hRsa);
    return 0;
}
