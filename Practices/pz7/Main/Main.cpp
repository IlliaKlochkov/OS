#include <windows.h>
#include <iostream>

using namespace std;

typedef int    (*pMultiply)(int, int);
typedef double (*pDivide)(double, double);

int wmain() {
    // Завантажуємо DLL динамічно — при цьому спрацює DllMain і перевірка CS
    HMODULE hDll = LoadLibraryW(L"MathDLL.dll");

    if (hDll == nullptr) {
        wcout << L"Cannot find MathDLL.dll: " << GetLastError() << endl;
        return 1;
    }

    wcout << L"DLL successfully loaded." << endl;
    // Отримуємо адреси функцій за їх іменами
    pMultiply MultiplyNumbers = (pMultiply)GetProcAddress(hDll, "MultiplyNumbers");
    pDivide   DivideNumbers = (pDivide)GetProcAddress(hDll, "DivideNumbers");

    if (!MultiplyNumbers || !DivideNumbers) {
        wcout << L"Cannot find functions. Error code: " << GetLastError() << endl;
        FreeLibrary(hDll);
        return 1;
    }

    wcout << L"MultiplyNumbers(6, 7) = " << MultiplyNumbers(6, 7) << endl;
    wcout << L"DivideNumbers(22, 7)  = " << DivideNumbers(22.0, 7.0) << endl;

    FreeLibrary(hDll);
	// чекаємо натискання клавіші перед завершенням програми
	wcout << L"Press Enter to exit..." << endl;
	cin.get();

    return 0;
} 