// Проєкт 1 - Створює текстові файли у цільовій теці. Підтримує ASCII та UNICODE (UTF-16 LE з BOM).
// Цільова тека: C:\lab7_files (за замовчуванням). Може бути змінена через змінну середовища OUTPUT_DIR або перший аргумент командного рядка.

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Допоміжна функція для негайного запуску процесу (чекає завершення)
static void RunWait(LPTSTR cmdLine)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}


// Допоміжна функція для відкладеного запуску процесу (не чекає)
static void RunNoWait(LPTSTR cmdLine, PROCESS_INFORMATION* out)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    
    if (out) {
        *out = pi;
    }
}

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
    HANDLE hFile = CreateFile(
        filePath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("  [ERROR] CreateFile failed: code %lu\n", GetLastError());
        return FALSE;
    }

    // Записати BOM для UTF-16 LE
    BYTE bom[2] = { 0xFF, 0xFE };
    DWORD written = 0;
    WriteFile(hFile, bom, 2, &written, NULL);

    // Записати вміст
    DWORD bytes = (DWORD)(wcslen(content) * sizeof(WCHAR));
    WriteFile(hFile, content, bytes, &written, NULL);
    CloseHandle(hFile);
    return TRUE;
}

static void OpenInNotepad(LPCTSTR filePath)
{
    TCHAR cmd[MAX_PATH * 2];
    
    _stprintf_s(cmd, MAX_PATH * 2, _T("notepad.exe \"%s\""), filePath);
    PROCESS_INFORMATION pi;
    RunNoWait(cmd, &pi);
    Sleep(800);

    if (pi.hProcess) {
        CloseHandle(pi.hProcess);
    }
    if (pi.hThread) {
        CloseHandle(pi.hThread);
    }
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
        TCHAR* env = NULL;
        size_t len = 0;
        if (_tdupenv_s(&env, &len, _T("OUTPUT_DIR")) == 0 && env)
        {
            _tcsncpy_s(outputDir, MAX_PATH, env, _TRUNCATE);
            free(env);
        }
    }

    printf("=== Program 1: Creating files ===\n");
    printf("Target directory: C:\\lab7_files\n");

    EnsureDirectory(outputDir);

    // --- ASCII файли ---
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

    // --- UNICODE файли (UTF-16 LE з BOM) ---
    TCHAR f3[MAX_PATH], f4[MAX_PATH];
    _stprintf_s(f3, MAX_PATH, _T("%s\\unicode_file1.txt"), outputDir);
    _stprintf_s(f4, MAX_PATH, _T("%s\\unicode_file2.txt"), outputDir);

    const WCHAR* uni1 =
        L"Лабораторна робота №7\r\n"
        L"Тема: Керування процесами\r\n"
        L"Виконавці: Кулик, Клочков, Калашник\r\n"
        L"Дата: 2026\r\n";

    const WCHAR* uni2 =
        L"Перший рядок Unicode файлу\r\n"
        L"Другий рядок з кирилицею\r\n"
        L"Третій рядок - завершальний\r\n";

    if (WriteUnicodeFile(f3, uni1))
        printf("  [OK] Unicode file created: unicode_file1.txt\n");
    if (WriteUnicodeFile(f4, uni2))
        printf("  [OK] Unicode file created: unicode_file2.txt\n");

    printf("\nOpening files in notepad...\n");
    OpenInNotepad(f1);
    OpenInNotepad(f2);
    OpenInNotepad(f3);
    OpenInNotepad(f4);

    printf("\nProgram 1 done. Files saved to: C:\\lab7_files\n");
    return 0;
}
