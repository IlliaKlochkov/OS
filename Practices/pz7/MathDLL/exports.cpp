#include <windows.h>

// Експортовані функції DLL — доступні зовнішнім програмам через GetProcAddress
extern "C" __declspec(dllexport) int MultiplyNumbers(int a, int b) {
    return a * b;
}

extern "C" __declspec(dllexport) double DivideNumbers(double a, double b) {
    // Захист від ділення на нуль
    if (b == 0.0) {
        OutputDebugStringW(L"[MathDLL] Attempt to divide by zero\n");
        return 0.0;
    }
    return a / b;
}