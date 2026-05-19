#include "IntegrityCheck.h"
#include <windows.h>
#include <iostream>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // lpReserved == nullptr ⇒ DLL завантажена через LoadLibrary (динамічно),
        // інакше — через імпорт-таблицю (статично)
        std::wcout << L"[rsa_lib_dyn DllMain] lpReserved = " << lpReserved
            << L" (" << (lpReserved == nullptr ? L"dynamic" : L"static") << L")\n";

        if (!VerifyDllChecksum(hModule))
            return FALSE;
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
