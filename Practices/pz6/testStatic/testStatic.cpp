#include <iostream>
#include "../mymath/mymath.h" // підключення заголовного файлу
#include <windows.h>

using namespace std;

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    long long a = 9223372036854775800LL; // майже ліміт (LLONG_MAX = 9223372036854775807)
    long long b = 100LL;
    long long res = 0;

    cout << "==================================================\n";
    cout << " Тест статичного завантаження mymath.dll          \n";
    cout << "==================================================\n";
    

    if (MyAdd(a, b, &res)) {
        cout << a << " + " << b << " = " << res << "\n";
    }
    else {
        cout << "Переповнення при додаванні: " << a << " + " << b << "\n";
    }

    if (MySub(1000, 500, &res)) {
        cout << "1000 - 500 = " << res << "\n";
    }

    if (MyDiv(a, 0, &res)) {
        cout << "Результат: " << res << "\n";
    }
    else {
        cout << "Помилка: Ділення на нуль!\n";
    }

    return 0;
}