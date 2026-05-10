#include <windows.h>
#include <cstdio>

// DllMain використовується тільки для ілюстрації параметра lpReserved
// (Lab 3, пункт 9): при статичному завантаженні lpReserved != nullptr,
// при динамічному (LoadLibrary) — nullptr
BOOL APIENTRY DllMain(HMODULE, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        wchar_t buf[128];
        swprintf_s(buf, L"[rsa_lib DllMain] lpReserved = %p (%s)\n",
            lpReserved, lpReserved == nullptr ? L"dynamic" : L"static");
        OutputDebugStringW(buf);
        // вивід в stdout додатку, що нас завантажив
        wprintf(L"%s", buf);
        break;
    }
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
