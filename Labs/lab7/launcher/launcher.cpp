// Проєкт 3 - Послідовно запускає Програму 1, потім Програму 2.
// Програма 1: нормальний пріоритет (через макрос RUN_PROCESS_WAIT).
// Програма 2: BELOW_NORMAL_PRIORITY_CLASS (через RunProcessWithPriority).
// Шляхи та тека передаються через змінні середовища або аргументи командного рядка: PROG1_PATH, PROG2_PATH, OUTPUT_DIR або argv[1], argv[2], argv[3]

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// макрос для негайного запуску процесу (чекає на завершення)
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
    else {
        printf("[ERROR] CreateProcess failed: code %lu\n", GetLastError());
    }
}

// макрос для відкладеного запуску процесу (не чекає)
static void RunNoWait(LPTSTR cmdLine, PROCESS_INFORMATION* out)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    if (out) *out = pi;
}


static BOOL RunProcessWithPriority(LPTSTR cmdLine, DWORD priorityClass)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    BOOL ok = CreateProcess(
        NULL, cmdLine, NULL, NULL, FALSE,
        priorityClass, NULL, NULL, &si, &pi
    );

    if (!ok)
    {
        printf("[ERROR] CreateProcess failed: code %lu\n", GetLastError());
        return FALSE;
    }

    printf("  Launched PID=%lu, priority class: 0x%lX\n",
        pi.dwProcessId, GetPriorityClass(pi.hProcess));

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;

    GetExitCodeProcess(pi.hProcess, &exitCode);
    printf("  Process exited with code: %lu\n", exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return TRUE;
}

int _tmain(int argc, TCHAR* argv[])
{
    TCHAR prog1[MAX_PATH] = _T("creator.exe");
    TCHAR prog2[MAX_PATH] = _T("analyzer.exe");
    TCHAR dir[MAX_PATH] = _T("C:\\lab7_files");

    if (argc >= 2) _tcsncpy_s(prog1, MAX_PATH, argv[1], _TRUNCATE);
    if (argc >= 3) _tcsncpy_s(prog2, MAX_PATH, argv[2], _TRUNCATE);
    if (argc >= 4) _tcsncpy_s(dir, MAX_PATH, argv[3], _TRUNCATE);

    if (argc < 2) {
        TCHAR* e = NULL;
        size_t l = 0;
        if (_tdupenv_s(&e, &l, _T("PROG1_PATH")) == 0 && e)
        {
            _tcsncpy_s(prog1, MAX_PATH, e, _TRUNCATE);
            free(e);
        }
    }
    if (argc < 3) {
        TCHAR* e = NULL;
        size_t l = 0;
        if (_tdupenv_s(&e, &l, _T("PROG2_PATH")) == 0 && e)
        {
            _tcsncpy_s(prog2, MAX_PATH, e, _TRUNCATE);
            free(e);
        }
    }
    if (argc < 4) {
        TCHAR* e = NULL;
        size_t l = 0;
        if (_tdupenv_s(&e, &l, _T("OUTPUT_DIR")) == 0 && e)
        {
            _tcsncpy_s(dir, MAX_PATH, e, _TRUNCATE);
            free(e);
        }
    }

    printf("=== Program 3: Sequential launch ===\n");
    printf("Program 1: creator.exe\n");
    printf("Program 2: analyzer.exe\n");
    printf("Directory: C:\\lab7_files\n");

    // запуск програми 1
    printf("\n[1/2] Launching Program 1 (NORMAL_PRIORITY_CLASS)...\n");
    TCHAR cmd1[MAX_PATH * 2];

    _stprintf_s(cmd1, MAX_PATH * 2, _T("%s %s"), prog1, dir);

    STARTUPINFO si1;
    PROCESS_INFORMATION pi1;

    RunWait(cmd1);
    printf("[1/2] Program 1 finished.\n");
    
    // запуск програми 2
    printf("\n[2/2] Launching Program 2 (BELOW_NORMAL_PRIORITY_CLASS)...\n");
    
    TCHAR cmd2[MAX_PATH * 2];

    _stprintf_s(cmd2, MAX_PATH * 2, _T("%s %s"), prog2, dir);
    RunProcessWithPriority(cmd2, BELOW_NORMAL_PRIORITY_CLASS);
    printf("[2/2] Program 2 finished.\n");

    printf("\n=== Program 3: all tasks done ===\n");
    return 0;
}