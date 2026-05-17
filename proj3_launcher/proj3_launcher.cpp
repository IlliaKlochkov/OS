// Project 3 - Launch Program 1 then Program 2 sequentially.
// Program 1: normal priority (via RUN_PROCESS_WAIT macro).
// Program 2: BELOW_NORMAL_PRIORITY_CLASS (via RunProcessWithPriority).
// Paths and folder are passed via env variables or command-line arguments:
//   PROG1_PATH, PROG2_PATH, OUTPUT_DIR
//   or argv[1], argv[2], argv[3]

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
        } else {                                                        \
            printf("[ERROR] CreateProcess failed: code %lu\n",          \
                   GetLastError());                                      \
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

static BOOL RunProcessWithPriority(LPTSTR cmdLine, DWORD priorityClass)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    BOOL ok = CreateProcess(NULL, cmdLine, NULL, NULL, FALSE,
        priorityClass, NULL, NULL, &si, &pi);
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
    TCHAR prog1[MAX_PATH] = _T("proj1_creator.exe");
    TCHAR prog2[MAX_PATH] = _T("proj2_analyzer.exe");
    TCHAR dir[MAX_PATH] = _T("C:\\lab7_files");

    if (argc >= 2) _tcsncpy_s(prog1, MAX_PATH, argv[1], _TRUNCATE);
    if (argc >= 3) _tcsncpy_s(prog2, MAX_PATH, argv[2], _TRUNCATE);
    if (argc >= 4) _tcsncpy_s(dir, MAX_PATH, argv[3], _TRUNCATE);

    if (argc < 2) {
        TCHAR* e = NULL; size_t l = 0;
        if (_tdupenv_s(&e, &l, _T("PROG1_PATH")) == 0 && e)
        {
            _tcsncpy_s(prog1, MAX_PATH, e, _TRUNCATE); free(e);
        }
    }
    if (argc < 3) {
        TCHAR* e = NULL; size_t l = 0;
        if (_tdupenv_s(&e, &l, _T("PROG2_PATH")) == 0 && e)
        {
            _tcsncpy_s(prog2, MAX_PATH, e, _TRUNCATE); free(e);
        }
    }
    if (argc < 4) {
        TCHAR* e = NULL; size_t l = 0;
        if (_tdupenv_s(&e, &l, _T("OUTPUT_DIR")) == 0 && e)
        {
            _tcsncpy_s(dir, MAX_PATH, e, _TRUNCATE); free(e);
        }
    }

    printf("=== Program 3: Sequential launch ===\n");
    printf("Program 1: proj1_creator.exe\n");
    printf("Program 2: proj2_analyzer.exe\n");
    printf("Directory: C:\\lab7_files\n");

    // Launch Program 1 - normal priority, wait for completion
    printf("\n[1/2] Launching Program 1 (NORMAL_PRIORITY_CLASS)...\n");
    TCHAR cmd1[MAX_PATH * 2];
    _stprintf_s(cmd1, MAX_PATH * 2, _T("%s %s"), prog1, dir);
    STARTUPINFO si1; PROCESS_INFORMATION pi1;
    RUN_PROCESS_WAIT(cmd1, si1, pi1);
    printf("[1/2] Program 1 finished.\n");

    // Launch Program 2 - below normal priority, wait for completion
    printf("\n[2/2] Launching Program 2 (BELOW_NORMAL_PRIORITY_CLASS)...\n");
    TCHAR cmd2[MAX_PATH * 2];
    _stprintf_s(cmd2, MAX_PATH * 2, _T("%s %s"), prog2, dir);
    RunProcessWithPriority(cmd2, BELOW_NORMAL_PRIORITY_CLASS);
    printf("[2/2] Program 2 finished.\n");

    printf("\n=== Program 3: all tasks done ===\n");
    return 0;
}