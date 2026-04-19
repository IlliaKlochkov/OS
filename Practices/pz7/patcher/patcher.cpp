#include <windows.h>
#include <stdio.h>
#include <tchar.h>

// алгоритм підрахунку контрольної суми (з прикладу поштової скриньки)
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

// ===============================
// |⣿⣿⣿⣿⣿⣿⣿⣿⣻⣿⣿⣿⡿⢿⡿⠿⠿⣿⣿⣿⣿⣿⣿⡿⣿⣿⣿⡿⣿|
// |⣿⣿⣿⠿⠿⢿⣿⣿⠟⣋⣭⣶⣶⣞⣿⣶⣶⣶⣬⣉⠻⣿⣿⡿⣋⣉⠻⣿⣿|
// |⢻⣿⠃⣤⣤⣢⣍⣴⣿⢋⣵⣿⣿⣿⣿⣷⣶⣝⣿⣿⣧⣄⢉⣜⣥⣜⢷⢹⢇|
// |⡦⡁⡸⢛⡴⢣⣾⢟⣿⣿⣿⢟⣾⣧⣙⢿⣿⣿⣿⣿⣿⣿⣿⢩⢳⣞⢿⡏⢷|
// |⣵⡇⣗⡾⢁⣾⣟⣾⣿⡿⣻⣾⣿⣿⣿⡎⠛⡛⢿⣿⡟⣿⣿⡜⡜⢿⡌⠇⢾|
// |⣿⠁⣾⠏⣾⣿⣿⣽⣑⣺⣥⣿⣿⣿⣿⣷⣶⣦⣖⢝⢿⣿⣿⣿⡀⠹⣿⣼⢸|
// |⣿⢰⡏⢡⣿⣿⠐⣵⠿⠛⠛⣿⣿⣿⣿⣿⠍⠚⢙⠻⢦⣼⣿⣿⠁⣄⣿⣿⠘|
// |⣿⢸⢹⢈⣿⣿⠘⣡⡞⠉⡀⢻⣿⣿⣿⣿⢃⠠⢈⢳⣌⣩⣿⣿⠰⠿⢼⣿⠀|
// |⠿⣘⠯⠌⡟⣿⡟⣾⣇⢾⡵⣹⣟⣿⣿⣿⣮⣓⣫⣿⣟⢿⣿⢿⡾⡹⢆⣦⣤|
// |⣛⠶⠽⣧⣋⠳⡓⢿⣿⣿⣿⣿⣿⢿⣿⣿⣿⣿⣿⣿⣫⣸⠏⡋⠷⣛⣫⡍⣶|
// ===============================
// |       функція main          |
// ===============================

int _tmain(int argc, _TCHAR* argv[]) {
    if (argc < 2) {
        _tprintf(_T("Використання: patcher.exe <шлях_до_DLL>\n"));
        return 1;
    }

    // відкриваємо файл DLL для читання та запису
    HANDLE hFile = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(_T("Помилка відкриття файлу: %d\n"), GetLastError());
        return 2;
    }

    DWORD dwSize = GetFileSize(hFile, NULL);
    PBYTE pMem = new BYTE[dwSize];
    DWORD dwCount;

    // читаємо весь файл
    if (!ReadFile(hFile, pMem, dwSize, &dwCount, NULL)) {
        _tprintf(_T("Помилка читання файлу!\n"));
        delete[] pMem;
        CloseHandle(hFile);
        return 3;
    }

    // рахуємо контрольну суму
    DWORD dwCS = GetControlSum(pMem, dwSize);

    // переміщуємо вказівник в кінець файлу та дописуємо 4 байти суми
    SetFilePointer(hFile, 0, NULL, FILE_END);
    if (WriteFile(hFile, &dwCS, sizeof(dwCS), &dwCount, NULL)) {
        _tprintf(_T("Контрольну суму (0x%08X) успішно записано в кінець файлу.\n"), dwCS);
    }
    else {
        _tprintf(_T("Помилка запису контрольної суми!\n"));
    }

    delete[] pMem;
    CloseHandle(hFile);
    return 0;
}