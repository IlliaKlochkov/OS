#include "IntegrityCheck.h"
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

// Той самий алгоритм, що і в CheckSumCalc — інакше суми не співпадуть
static DWORD ComputeChecksum(const BYTE* data, DWORD size) {
    DWORD checksum = 0;
    DWORD fullDwords = size / sizeof(DWORD);
    const DWORD* dwordPtr = reinterpret_cast<const DWORD*>(data);

    for (DWORD i = 0; i < fullDwords; ++i)
        checksum += dwordPtr[i];

    DWORD remaining = size % sizeof(DWORD);
    if (remaining > 0) {
        DWORD tail = 0;
        memcpy(&tail, data + fullDwords * sizeof(DWORD), remaining);
        checksum += tail;
    }
    return checksum;
}

static void Log(const wchar_t* msg) {
    OutputDebugStringW(msg);
    wcout << msg;
}

bool VerifyDllChecksum(HMODULE hModule) {
    wchar_t dllPath[MAX_PATH] = {};
    DWORD len = GetModuleFileNameW(hModule, dllPath, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        Log(L"[Integrity] Cannot get DLL path\n");
        return false;
    }

    wcout << L"[Integrity] File: " << dllPath << endl;
    HANDLE hFile = CreateFileW(dllPath, GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        Log(L"[Integrity] Cannot open DLL file\n");
        return false;
    }

    DWORD fileSize = GetFileSize(hFile, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize < sizeof(DWORD)) {
        Log(L"[Integrity] File size is invalid\n");
        CloseHandle(hFile);
        return false;
    }

    vector<BYTE> buffer(fileSize);
    DWORD bytesRead = 0;
    SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
    bool ok = ReadFile(hFile, buffer.data(), fileSize, &bytesRead, nullptr);
    CloseHandle(hFile);

    if (!ok || bytesRead != fileSize) {
        Log(L"[Integrity] File read error\n");
        return false;
    }

    DWORD stored = 0;
    memcpy(&stored, buffer.data() + fileSize - sizeof(DWORD), sizeof(DWORD));
    DWORD calculated = ComputeChecksum(buffer.data(), fileSize - sizeof(DWORD));

    if (calculated != stored) {
        Log(L"[Integrity] Checksum mismatch - file is corrupted\n");
        return false;
    }

    Log(L"[Integrity] Checksum is valid\n");
    return true;
}
