#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

// ==========================================
// Задача 1
// ==========================================

BOOL сompareDirectories(const TCHAR* dir1, const TCHAR* dir2) {
    TCHAR temp1[MAX_PATH];
    TCHAR temp2[MAX_PATH];

    // копіюємо рядки для безпечної модифікації
    _tcscpy(temp1, dir1);
    _tcscpy(temp2, dir2);

    // видаляємо '\' в кінці першого шляху, якщо він є
    size_t len1 = _tcslen(temp1);
    if (len1 > 0 && temp1[len1 - 1] == _T('\\')) {
        temp1[len1 - 1] = _T('\0');
    }

    // видаляємо '\' в кінці другого шляху, якщо він є
    size_t len2 = _tcslen(temp2);
    if (len2 > 0 && temp2[len2 - 1] == _T('\\')) {
        temp2[len2 - 1] = _T('\0');
    }

    // порівнюємо без урахування регістру (аналог wcsicmp / stricmp для TCHAR)
    return _tcsicmp(temp1, temp2) == 0;
}

void runTask1() {
    TCHAR dirA[] = _T("C:\\Windows\\System32\\");
    TCHAR dirB[] = _T("c:\\windows\\system32");
    TCHAR dirC[] = _T("c:\\windows\\system32\\temp");

    _tprintf(_T("Директорія A: %s\n"), dirA);
    _tprintf(_T("Директорія B: %s\n"), dirB);
    _tprintf(_T("Директорія C: %s\n"), dirC);

    _tprintf(_T("\nРезультати порівняння:\n"));

    // Порівняння 1: A та B
    if (сompareDirectories(dirA, dirB)) {
        _tprintf(_T("Директорії A і B збігаються\n"));
    }
    else {
        _tprintf(_T("Директорії A і B не збігаються\n"));
    }

    // Порівняння 2: A та C
    if (сompareDirectories(dirA, dirC)) {
        _tprintf(_T("Директорії A і C збігаються\n"));
    }
    else {
        _tprintf(_T("Директорії A і C не збігаються\n"));
    }
}

// ==========================================
// Задача 2
// ==========================================

void sortStringsKnown(TCHAR* arr[], int count) {
    // сортування бульбашкою
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            // _tcscmp виконує порівняння з урахуванням регістру 
            if (_tcscmp(arr[j], arr[j + 1]) > 0) {
                TCHAR* temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}


void runTask2() {
    TCHAR str1[] = _T("Banana");
    TCHAR str2[] = _T("Apple");
    TCHAR str3[] = _T("Cherry");
    TCHAR* arr[] = { str1, str2, str3 };

    _tprintf(_T("Початковий масив:\n"));
    for (int i = 0; i < 3; i++) {
        _tprintf(_T("%s\n"), arr[i]);
    }

    sortStringsKnown(arr, 3);

    _tprintf(_T("\nВідсортований масив:\n"));
    for (int i = 0; i < 3; i++) {
        _tprintf(_T("%s\n"), arr[i]);
    }
}

// ==========================================
// Задача 3
// ==========================================

// допоміжна функція яка амість BOOL повертає int (<0, 0, >0) для можливості сортування.
int compareTextsForSort(PVOID Text1, size_t n1, PVOID Text2, size_t n2) {
    int iPriz1 = -1, iPriz2 = -1;
    BOOL b1 = IsTextUnicode(Text1, (int)n1, &iPriz1);
    BOOL b2 = IsTextUnicode(Text2, (int)n2, &iPriz2);

    wchar_t* p1 = 0, * p2 = 0;

    // якщо рядок 1 не UNICODE, перетворюємо його
    if (!b1) {
        p1 = new wchar_t[n1];
        MultiByteToWideChar(CP_ACP, 0, (char*)Text1, (int)n1, p1, (int)n1);
    }
    else {
        p1 = (wchar_t*)Text1;
    }

    // якщо рядок 2 не UNICODE, перетворюємо його
    if (!b2) {
        p2 = new wchar_t[n2];
        MultiByteToWideChar(CP_ACP, 0, (char*)Text2, (int)n2, p2, (int)n2);
    }
    else {
        p2 = (wchar_t*)Text2;
    }

    // порівняння рядків в UNICODE 
    int res = wcscmp(p1, p2);

    // визволення пам’яті, якщо відбулося перетворення
    if (!b1) delete[] p1;
    if (!b2) delete[] p2;

    return res;
}

// структура для зручного збереження даних невідомого типу та розміру буфера
struct UnknownString {
    PVOID data;
    size_t size;
};

// функція сортування масиву рядків з невідомим кодуванням
void sortStringsUnknown(UnknownString arr[], int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (compareTextsForSort(arr[j].data, arr[j].size, arr[j + 1].data, arr[j + 1].size) > 0) {
                UnknownString temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}


void runTask3() {
    // симулюємо масив з різним кодуванням
    char asciiStr[] = "Zebra";
    wchar_t wideStr[] = L"Мавпа";
    char asciiStr2[] = "Elephant";

    // запаковуємо у структури 
    UnknownString arr[] = {
        { (PVOID)asciiStr, sizeof(asciiStr) },
        { (PVOID)wideStr, sizeof(wideStr) },
        { (PVOID)asciiStr2, sizeof(asciiStr2) }
    };

    _tprintf(_T("Вхідні дані:\n"));
    for (int i = 0; i < 3; i++) {
        int priz = -1;
        if (IsTextUnicode(arr[i].data, (int)arr[i].size, &priz)) {
            wprintf(L"Unicode: %s\n", (wchar_t*)arr[i].data);
        }
        else {
            printf("ASCII: %s\n", (char*)arr[i].data);
        }
    }

    sortStringsUnknown(arr, 3);

    _tprintf(_T("\nВідсортовані дані:\n"));
    // вивід результатів сортування змішаного масиву
    for (int i = 0; i < 3; i++) {
        int priz = -1;
        if (IsTextUnicode(arr[i].data, (int)arr[i].size, &priz)) {
            wprintf(L"Unicode: %s\n", (wchar_t*)arr[i].data);
        }
        else {
            printf("ASCII: %s\n", (char*)arr[i].data);
        }
    }
}


// ===============================
// |⣿⣿⣿⣿⣿⣿⣿⣿⣻⣿⣿⣿⡿⢿⡿⠿⠿⣿⣿⣿⣿⣿⣿⡿⣿⣿⣿⡿⣿|
// |⣿⣿⣿⠿⠿⢿⣿⣿⠟⣋⣭⣶⣶⣞⣿⣶⣶⣶⣬⣉⠻⣿⣿⡿⣋⣉⠻⣿⣿|
// |⢻⣿⠃⣤⣤⣢⣍⣴⣿⢋⣵⣿⣿⣿⣿⣷⣶⣝⣿⣿⣧⣄⢉⣜⣥⣜⢷⢹⢇|
// |⡦⡁⡸⢛⡴⢣⣾⢟⣿⣿⣿⢟⣾⣧⣙⢿⣿⣿⣿⣿⣿⣿⣿⢩⢳⣞⢿⡏⢷|
// |⣵⡇⣗⡾⢁⣾⣟⣾⣿⡿⣻⣾⣿⣿⣿⡎⠛⡛⢿⣿⡟⣿⣿⡜⡜⢿⡌⠇⢾|
// |⣿⠁⣾⠏⣾⣿⣿⣽⣑⣺⣥⣿⣿⣿⣿⣷⣶⣦⣖⢝⢿⣿⣿⣿⡀⠹⣿⣼⢸|
// |⣿⢰⡏⢡⣿⣿⠐⣵⠿⠛⠛⣿⣿⣿⣿⣿⠍⠚⢙⠻⢦⣼⣿⣿⠁⣄⣿⣿⠘|
// |⣿⢸⢹⢈⣿⣿⠘⣡⡞⠉⡀⢻⣿⣿⣿⣿⢃⠠⢈⢳⣌⣩⣿⣿⠰⠿⢼⣿⠀|
// |⠿⣘⠯⠌⡟⣿⡟⣾⣇⢾⡵⣹⣟⣿⣿⣿⣮⣓⣫⣿⣟⢿⣿⢿⡾⡹⢆⣦⣤|
// |⣛⠶⠽⣧⣋⠳⡓⢿⣿⣿⣿⣿⣿⢿⣿⣿⣿⣿⣿⣿⣫⣸⠏⡋⠷⣛⣫⡍⣶|
// ===============================
// |       функція main          |
// ===============================

// просто допоміжна функція для зручного виводу
void printSeparator(const TCHAR* title)
{
    _tprintf(_T("\n==================================================\n"));
    _tprintf(_T("%s\n"), title);
    _tprintf(_T("==================================================\n"));
}


int _tmain(int argc, TCHAR* argv[])
{
    setlocale(LC_ALL, "Ukrainian");

    printSeparator(_T("Завдання 1"));
    runTask1();
    
    printSeparator(_T("Завдання 2"));
    runTask2();

    printSeparator(_T("Завдання 3"));
    runTask3();

    return 0;
}