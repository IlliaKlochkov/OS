#include "IntegrityCheck.h"
#include <windows.h>
#include <iostream>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // lpReserved == nullptr означає динамічне завантаження (LoadLibrary),
        // інакше — статичне (через .lib)
        if (lpReserved == nullptr)
            OutputDebugStringW(L"[DllMain] Dynamic loading\n");
        else
            OutputDebugStringW(L"[DllMain] Static loading\n");

        // Якщо контрольна сума не пройшла — забороняємо завантаження DLL
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