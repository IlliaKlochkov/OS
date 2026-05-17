// Проєкт 2 - Аналізує текстові файли у цільовій теці.Визначає: розмір файлу, кількість рядків, довжину кожного рядка.

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <tlhelp32.h>

// Макрос для негайного запуску процесу (чекає завершення)
static void RunWait(LPTSTR cmdLine)
{
    STARTUPINFO si; PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    }
}


// Макрос для відкладеного запуску процесу (не чекає)
static void RunNoWait(LPTSTR cmdLine, PROCESS_INFORMATION* out)
{
    STARTUPINFO si; PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (out) *out = pi;
}

typedef enum { ENC_ANSI, ENC_UNICODE_UTF16LE } FileEncoding;

static FileEncoding DetectEncoding(HANDLE hFile)
{
    BYTE bom[2] = { 0, 0 };
    DWORD read = 0;
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    ReadFile(hFile, bom, 2, &read, NULL);
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    if (read == 2 && bom[0] == 0xFF && bom[1] == 0xFE)
        return ENC_UNICODE_UTF16LE;
    return ENC_ANSI;
}

static void AnalyzeAnsiFile(HANDLE hFile, DWORD fileSize)
{
    if (fileSize == 0) { printf("  [empty file]\n"); return; }

    char* buf = (char*)malloc(fileSize + 1);
    if (!buf) return;
    DWORD read = 0;
    ReadFile(hFile, buf, fileSize, &read, NULL);
    buf[read] = '\0';

    int lineCount = 0, lineStart = 0;
    for (DWORD i = 0; i <= read; i++)
    {
        if (i == read || buf[i] == '\n')
        {
            int end = (int)i;
            if (end > lineStart && buf[end - 1] == '\r') end--;
            int len = end - lineStart;
            if (i == read && len == 0) break;
            lineCount++;
            printf("  Line %d: length %d chars\n", lineCount, len);
            lineStart = (int)i + 1;
        }
    }
    printf("  Total lines: %d\n", lineCount);
    free(buf);
}

static void AnalyzeUnicodeFile(HANDLE hFile, DWORD fileSize)
{
    if (fileSize <= 2) { printf("  [empty file]\n"); return; }

    SetFilePointer(hFile, 2, NULL, FILE_BEGIN);
    DWORD wCount = (fileSize - 2) / sizeof(WCHAR);
    WCHAR* buf = (WCHAR*)malloc((wCount + 1) * sizeof(WCHAR));
    if (!buf) return;
    DWORD read = 0;
    ReadFile(hFile, buf, wCount * sizeof(WCHAR), &read, NULL);
    buf[wCount] = L'\0';

    int lineCount = 0, lineStart = 0;
    for (DWORD i = 0; i <= wCount; i++)
    {
        if (i == wCount || buf[i] == L'\n')
        {
            int end = (int)i;
            if (end > lineStart && buf[end - 1] == L'\r') end--;
            int len = end - lineStart;
            if (i == wCount && len == 0) break;
            lineCount++;
            printf("  Line %d: length %d chars\n", lineCount, len);
            lineStart = (int)i + 1;
        }
    }
    printf("  Total lines: %d\n", lineCount);
    free(buf);
}

static void AnalyzeFile(LPCTSTR filePath)
{
    // Короткочасно вивести ім'я файлу у широкому форматі, потім відновити режим
    printf("\n--- File: ");
    int old = _setmode(_fileno(stdout), _O_U16TEXT);
    wprintf(L"%s", filePath);
    _setmode(_fileno(stdout), old);
    printf(" ---\n");

    HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("  [ERROR] Cannot open file: code %lu\n", GetLastError());
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    printf("  Size: %lu bytes\n", fileSize);

    FileEncoding enc = DetectEncoding(hFile);
    if (enc == ENC_UNICODE_UTF16LE)
    {
        printf("  Encoding: Unicode UTF-16 LE\n");
        AnalyzeUnicodeFile(hFile, fileSize);
    }
    else
    {
        printf("  Encoding: ASCII / ANSI\n");
        AnalyzeAnsiFile(hFile, fileSize);
    }

    CloseHandle(hFile);
}

static void ListRunningProcesses(void)
{
    printf("\n=== Running processes ===\n");
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
    {
        printf("[ERROR] CreateToolhelp32Snapshot: code %lu\n", GetLastError());
        return;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnap, &pe))
    {
        printf("[ERROR] Process32First: code %lu\n", GetLastError());
        CloseHandle(hSnap);
        return;
    }
    int count = 0;
    do
    {
        count++;
        printf("[%3d] PID: %-6lu\n", count, pe.th32ProcessID);
    } while (Process32Next(hSnap, &pe));

    printf("Total processes: %d\n", count);
    CloseHandle(hSnap);
}

static void ListProcessModules(DWORD pid)
{
    printf("\n=== Modules of current process (PID=%lu) ===\n", pid);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    MODULEENTRY32 me;
    me.dwSize = sizeof(MODULEENTRY32);
    if (Module32First(hSnap, &me))
    {
        do
        {
            // Вивести шлях модуля у широкому форматі
            int old = _setmode(_fileno(stdout), _O_U16TEXT);
            wprintf(L"  %s\n", me.szExePath);
            _setmode(_fileno(stdout), old);
        } while (Module32Next(hSnap, &me));
    }
    CloseHandle(hSnap);
}

int _tmain(int argc, TCHAR* argv[])
{
    TCHAR outputDir[MAX_PATH] = _T("C:\\lab7_files");

    if (argc >= 2)
    {
        _tcsncpy_s(outputDir, MAX_PATH, argv[1], _TRUNCATE);
    }
    else
    {
        TCHAR* env = NULL; size_t len = 0;
        if (_tdupenv_s(&env, &len, _T("OUTPUT_DIR")) == 0 && env)
        {
            _tcsncpy_s(outputDir, MAX_PATH, env, _TRUNCATE);
            free(env);
        }
    }

    printf("=== Program 2: File analysis ===\n");
    printf("Search directory: C:\\lab7_files\n");

    TCHAR pattern[MAX_PATH];
    _stprintf_s(pattern, MAX_PATH, _T("%s\\*.txt"), outputDir);

    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("No files found. Run Program 1 first.\n");
    }
    else
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                TCHAR fullPath[MAX_PATH];
                _stprintf_s(fullPath, MAX_PATH, _T("%s\\%s"), outputDir, fd.cFileName);
                AnalyzeFile(fullPath);
            }
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
    }

    ListRunningProcesses();

    DWORD pid = GetCurrentProcessId();
    ListProcessModules(pid);

    printf("\nProgram 2 done.\n");
    return 0;
}