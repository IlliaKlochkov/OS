#pragma once

#ifdef THREAD_LIB_EXPORTS
#define THREAD_API __declspec(dllexport)
#else
#define THREAD_API __declspec(dllimport)
#endif

#include <windows.h>

// параметр потокової функції
struct ThreadParam {
    int  id;          // номер потоку
    int  iterations;  // кількість ітерацій у циклі
    volatile long* counter; // лічильник викликів (загальний)
};

// потокова функція
extern "C" THREAD_API DWORD WINAPI ThreadFunc(LPVOID param);