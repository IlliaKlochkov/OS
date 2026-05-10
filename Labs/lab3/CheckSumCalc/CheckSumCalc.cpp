#include <windows.h>
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

// 4-байтні DWORD-блоки сумуються; залишок (якщо є) додається окремо.
DWORD ComputeChecksum(const BYTE* data, DWORD size) {
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

// Якщо в кінці файлу вже є валідна CS — замінює, інакше дописує
bool WriteChecksumToFile(const wchar_t* filePath) {
    wcout << L"File: " << filePath << endl;

    HANDLE hFile = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        wcerr << L"Cannot open file. Error code: " << GetLastError() << endl;
        return false;
    }

    DWORD fileSize = GetFileSize(hFile, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        wcerr << L"File is empty or invalid." << endl;
        CloseHandle(hFile);
        return false;
    }

    wcout << L"File size: " << fileSize << L" bytes" << endl;

    vector<BYTE> buffer(fileSize);
    DWORD bytesRead = 0;
    SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
    ReadFile(hFile, buffer.data(), fileSize, &bytesRead, nullptr);

    DWORD usefulSize = fileSize;
    if (fileSize >= sizeof(DWORD)) {
        DWORD stored = 0;
        memcpy(&stored, buffer.data() + fileSize - sizeof(DWORD), sizeof(DWORD));
        DWORD calc = ComputeChecksum(buffer.data(), fileSize - sizeof(DWORD));
        if (stored == calc) {
            usefulSize = fileSize - sizeof(DWORD);
            wcout << L"Old checksum found - it will be replaced." << endl;
        }
    }

    DWORD newChecksum = ComputeChecksum(buffer.data(), usefulSize);

    SetFilePointer(hFile, usefulSize, nullptr, FILE_BEGIN);
    SetEndOfFile(hFile);

    DWORD bytesWritten = 0;
    WriteFile(hFile, &newChecksum, sizeof(DWORD), &bytesWritten, nullptr);
    CloseHandle(hFile);

    wcout << L"Checksum written: " << dec << newChecksum << endl;
    return true;
}

// Перевіряє контрольну суму без модифікації файлу — для діагностики
bool VerifyChecksum(const wchar_t* filePath) {
    HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        wcerr << L"Cannot open file. Error code: " << GetLastError() << endl;
        return false;
    }
    DWORD fileSize = GetFileSize(hFile, nullptr);
    if (fileSize < sizeof(DWORD)) { CloseHandle(hFile); return false; }

    vector<BYTE> buffer(fileSize);
    DWORD bytesRead = 0;
    ReadFile(hFile, buffer.data(), fileSize, &bytesRead, nullptr);
    CloseHandle(hFile);

    DWORD stored = 0;
    memcpy(&stored, buffer.data() + fileSize - sizeof(DWORD), sizeof(DWORD));
    DWORD calc = ComputeChecksum(buffer.data(), fileSize - sizeof(DWORD));

    wcout << L"Stored:     " << stored << endl;
    wcout << L"Calculated: " << calc << endl;
    bool ok = (stored == calc);
    wcout << (ok ? L"VALID" : L"MISMATCH") << endl;
    return ok;
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        wcerr << L"Using: CheckSumCalc.exe <file_path> [--verify]" << endl;
        return 1;
    }
    if (argc >= 3 && wcscmp(argv[2], L"--verify") == 0)
        return VerifyChecksum(argv[1]) ? 0 : 1;
    return WriteChecksumToFile(argv[1]) ? 0 : 1;
}
