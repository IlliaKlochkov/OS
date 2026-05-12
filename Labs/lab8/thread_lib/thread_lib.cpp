#define THREAD_LIB_EXPORTS
#include "thread_lib.h"
#include <windows.h>
#include <stdio.h>

// критична секція для захисту запису у лог-файл
static CRITICAL_SECTION g_logCS;
static bool             g_csInit = false;

// ініціалізація критичної секції при завантаженні DLL
BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        InitializeCriticalSection(&g_logCS);
        g_csInit = true;
    }
    if (reason == DLL_PROCESS_DETACH && g_csInit) {
        DeleteCriticalSection(&g_logCS);
    }
    return TRUE;
}

// запис рядка у debug_log.txt (тільки в DEBUG-збірці)
static void DebugLog(const char* msg) {
#ifdef _DEBUG
    EnterCriticalSection(&g_logCS);
    FILE* f = nullptr;
    fopen_s(&f, "debug_log.txt", "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
    LeaveCriticalSection(&g_logCS);
#else
    (void)msg;
#endif
}

// потокова функція
DWORD WINAPI ThreadFunc(LPVOID param) {
    ThreadParam* p = reinterpret_cast<ThreadParam*>(param);

    char buf[128];
    sprintf_s(buf, "Thread %d START", p->id);
    DebugLog(buf);

    // цикл, що займає більше одного кванта часу
    volatile long long dummy = 0;
    for (int i = 0; i < p->iterations; i++)
        dummy += i;

    // атомарне збільшення лічильника викликів
    InterlockedIncrement(p->counter);

    sprintf_s(buf, "Thread %d END (dummy=%lld)", p->id, dummy);
    DebugLog(buf);

    return 0;
}