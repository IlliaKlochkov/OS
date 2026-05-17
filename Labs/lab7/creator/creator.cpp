// Project 1 - Create text files in the target folder.
// Supports ASCII and UNICODE (UTF-16 LE with BOM) formats.
// Target folder: C:\lab7_files (default)
// Can be changed via OUTPUT_DIR env variable or first command-line argument.

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Macro for immediate process launch (waits for completion)
#define RUN_PROCESS_WAIT(cmdLine, si, pi)                              \
    do {                                                                \
        ZeroMemory(&(si), sizeof(si)); (si).cb = sizeof(si);           \
        ZeroMemory(&(pi), sizeof(pi));                                  \
        if (CreateProcess(NULL, (cmdLine), NULL, NULL, FALSE, 0,        \
                          NULL, NULL, &(si), &(pi))) {                  \
            WaitForSingleObject((pi).hProcess, INFINITE);               \
            CloseHandle((pi).hProcess); CloseHandle((pi).hThread);      \
        }                                                               \
    } while(0)

// Macro for deferred process launch (does not wait)
#define RUN_PROCESS_NOWAIT(cmdLine, si, pi)                            \
    do {                                                                \
        ZeroMemory(&(si), sizeof(si)); (si).cb = sizeof(si);           \
        ZeroMemory(&(pi), sizeof(pi));                                  \
        CreateProcess(NULL, (cmdLine), NULL, NULL, FALSE, 0,            \
                      NULL, NULL, &(si), &(pi));                        \
    } while(0)

static void EnsureDirectory(LPCTSTR path)
{
    if (!CreateDirectory(path, NULL))
    {
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS)
            printf("  [ERROR] CreateDirectory failed: code %lu\n", err);
    }
}

static BOOL WriteAnsiFile(LPCTSTR filePath, const char* content)
{
    HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("  [ERROR] CreateFile failed: code %lu\n", GetLastError());
        return FALSE;
    }
    DWORD written = 0;
    WriteFile(hFile, content, (DWORD)strlen(content), &written, NULL);
    CloseHandle(hFile);
    return TRUE;
}

static BOOL WriteUnicodeFile(LPCTSTR filePath, LPCWSTR content)
{
    HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("  [ERROR] CreateFile failed: code %lu\n", GetLastError());
        return FALSE;
    }
    // Write UTF-16 LE BOM
    BYTE bom[2] = { 0xFF, 0xFE };
    DWORD written = 0;
    WriteFile(hFile, bom, 2, &written, NULL);
    // Write content
    DWORD bytes = (DWORD)(wcslen(content) * sizeof(WCHAR));
    WriteFile(hFile, content, bytes, &written, NULL);
    CloseHandle(hFile);
    return TRUE;
}

static void OpenInNotepad(LPCTSTR filePath)
{
    TCHAR cmd[MAX_PATH * 2];
    _stprintf_s(cmd, MAX_PATH * 2, _T("notepad.exe %s"), filePath);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    RUN_PROCESS_NOWAIT(cmd, si, pi);
    Sleep(800);
    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread)  CloseHandle(pi.hThread);
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

    printf("=== Program 1: Creating files ===\n");
    printf("Target directory: C:\\lab7_files\n");

    EnsureDirectory(outputDir);

    // --- ASCII files ---
    TCHAR f1[MAX_PATH], f2[MAX_PATH];
    _stprintf_s(f1, MAX_PATH, _T("%s\\ansi_file1.txt"), outputDir);
    _stprintf_s(f2, MAX_PATH, _T("%s\\ansi_file2.txt"), outputDir);

    const char* ansi1 =
        "Lab work 7\r\n"
        "Topic: Process management\r\n"
        "Authors: Kulyk Yevhen, Klochkov Illia, Kalashnyk Andrii\r\n"
        "Year: 2026\r\n";

    const char* ansi2 =
        "First line of ASCII file\r\n"
        "Second line\r\n"
        "Third line with data\r\n";

    if (WriteAnsiFile(f1, ansi1))
        printf("  [OK] ASCII file created: ansi_file1.txt\n");
    if (WriteAnsiFile(f2, ansi2))
        printf("  [OK] ASCII file created: ansi_file2.txt\n");

    // --- UNICODE files (UTF-16 LE with BOM) ---
    TCHAR f3[MAX_PATH], f4[MAX_PATH];
    _stprintf_s(f3, MAX_PATH, _T("%s\\unicode_file1.txt"), outputDir);
    _stprintf_s(f4, MAX_PATH, _T("%s\\unicode_file2.txt"), outputDir);

    const WCHAR* uni1 =
        L"\u041b\u0430\u0431\u043e\u0440\u0430\u0442\u043e\u0440\u043d\u0430 \u0440\u043e\u0431\u043e\u0442\u0430 \u21167\r\n"
        L"\u0422\u0435\u043c\u0430: \u041a\u0435\u0440\u0443\u0432\u0430\u043d\u043d\u044f \u043f\u0440\u043e\u0446\u0435\u0441\u0430\u043c\u0438\r\n"
        L"\u0412\u0438\u043a\u043e\u043d\u0430\u0432\u0446\u0456: \u041a\u0443\u043b\u0438\u043a, \u041a\u043b\u043e\u0447\u043a\u043e\u0432, \u041a\u0430\u043b\u0430\u0448\u043d\u0438\u043a\r\n"
        L"\u0414\u0430\u0442\u0430: 2026\r\n";

    const WCHAR* uni2 =
        L"\u041f\u0435\u0440\u0448\u0438\u0439 \u0440\u044f\u0434\u043e\u043a Unicode \u0444\u0430\u0439\u043b\u0443\r\n"
        L"\u0414\u0440\u0443\u0433\u0438\u0439 \u0440\u044f\u0434\u043e\u043a \u0437 \u043a\u0438\u0440\u0438\u043b\u0438\u0446\u0435\u044e\r\n"
        L"\u0422\u0440\u0435\u0442\u0456\u0439 \u0440\u044f\u0434\u043e\u043a \u2013 \u0437\u0430\u0432\u0435\u0440\u0448\u0430\u043b\u044c\u043d\u0438\u0439\r\n";

    if (WriteUnicodeFile(f3, uni1))
        printf("  [OK] Unicode file created: unicode_file1.txt\n");
    if (WriteUnicodeFile(f4, uni2))
        printf("  [OK] Unicode file created: unicode_file2.txt\n");

    // Open each file in Notepad (deferred launch)
    printf("\nOpening files in notepad...\n");
    OpenInNotepad(f1);
    OpenInNotepad(f2);
    OpenInNotepad(f3);
    OpenInNotepad(f4);

    printf("\nProgram 1 done. Files saved to: C:\\lab7_files\n");
    return 0;
}