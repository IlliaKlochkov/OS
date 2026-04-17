#include "pch.h"
#include <windows.h>
#include <tchar.h>

// той самий алгоритм підрахунку для перевірки що і в patcher
DWORD GetControlSum(PBYTE pMem, DWORD dwCount) {
    DWORD dwCS = 0;
    DWORD dwFull = dwCount / 4;
    PDWORD p32Mem = (PDWORD)pMem;
    for (DWORD i = 0; i < dwFull; ++i)
        dwCS += p32Mem[i];
    DWORD dwNotFull = dwCount % 4;
    if (dwNotFull) {
        pMem += dwFull * 4;
        DWORD dwNumber = 0;
        memcpy(&dwNumber, pMem, dwNotFull);
        dwCS += dwNumber;
    }
    return dwCS;
}

// функція перевірки контрольної суми
BOOL VerifyChecksum(HMODULE hModule) {
    TCHAR szPath[MAX_PATH];
    
    // отримуємо повний шлях до поточної DLL
    GetModuleFileName(hModule, szPath, MAX_PATH);

    // відкриваємо саму себе для читання
    HANDLE hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    DWORD dwSize = GetFileSize(hFile, NULL);
    
    // якщо файл менший за розмір самої суми, щось не так
    if (dwSize <= sizeof(DWORD)) {
        CloseHandle(hFile);
        return FALSE;
    }

    PBYTE pMem = new BYTE[dwSize];
    DWORD dwRead;
    ReadFile(hFile, pMem, dwSize, &dwRead, NULL);
    CloseHandle(hFile);

    // останні 4 байти - це записана контрольна сума
    DWORD storedCS = *(PDWORD)(pMem + dwSize - sizeof(DWORD));

    // рахуємо контрольну суму контенту без останніх 4 байт
    DWORD calculatedCS = GetControlSum(pMem, dwSize - sizeof(DWORD));

    delete[] pMem;

    return (storedCS == calculatedCS);
}

// головна функція DLL
BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        // вставляємо перевірку при завантаженні
        if (!VerifyChecksum(hModule)) {
            
            // якщо суми не збігаються, блокуємо завантаження
            MessageBox(NULL, _T("Помилка: Контрольна сума DLL не збігається! Файл пошкоджено або змінено."), _T("Помилка безпеки"), MB_ICONERROR);
            return FALSE;
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}