#include <iostream>
#include <windows.h>

using namespace std;

// визначення типу вказівника на функцію для коректного виклику за адресою
typedef bool (*TMATH64)(long long, long long, long long*);

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    cout << "==================================================\n";
    cout << " Тест статичного завантаження mymath.dll          \n";
    cout << "==================================================\n";

    // завантажуємо саму DLL 
    HMODULE hLib = LoadLibrary(TEXT("mymath.dll"));

    if (hLib != NULL) {
        // отримання адрес функцій за їхніми іменами
        TMATH64 AddFunc = (TMATH64)GetProcAddress(hLib, "MyAdd");
        TMATH64 MulFunc = (TMATH64)GetProcAddress(hLib, "MyMul");

        if (AddFunc && MulFunc) {
            long long a = 15000;
            long long b = 3;
            long long res;

            // виклик функцій через отримані вказівники
            if (AddFunc(a, b, &res)) {
                cout << a << " + " << b << " = " << res << " (через DLL)\n";
            }
            if (MulFunc(a, b, &res)) {
                cout << a << " * " << b << " = " << res << " (через DLL)\n";
            }
        }
        else {
            cout << "Не вдалося знайти потрібні функції в бібліотеці\n";
        }

        // вивантаження бібліотеки для звільнення ресурсів
        FreeLibrary(hLib);
        cout << "\nБібліотеку успішно вивантажено з пам'яті\n";
    }
    else {
        cout << "Помилка: не вдалося завантажити mymath.dll!\n";
    }

    return 0;
}