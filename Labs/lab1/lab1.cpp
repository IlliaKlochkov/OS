#include <windows.h>
#include <tchar.h>
#include <locale.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <clocale>
#include <io.h>
#include <fcntl.h>

using namespace std;


enum class FileEncoding
{
    ANSI_TEXT,
    UNICODE_UTF16_LE
};


// ======================================================
// Допоміжні функції
// ======================================================

void setupLocale()
{
    if (_tsetlocale(LC_ALL, _T("Ukrainian")) == NULL)
    {
        _tprintf(_T("Не вдалося встановити локаль Ukrainian.\n"));
    }

    setlocale(LC_ALL, "Ukrainian");
}

void printSeparator(const TCHAR* title)
{
    _tprintf(_T("\n==================================================\n"));
    _tprintf(_T("%s\n"), title);
    _tprintf(_T("==================================================\n"));
}

void printEncodingInfo()
{
    printSeparator(_T("1. Перевірка типу кодування за замовчуванням"));

    _tprintf(_T("sizeof(TCHAR) = %d байт(и)\n"), (int)sizeof(TCHAR));

    if (sizeof(TCHAR) == sizeof(char))
        _tprintf(_T("Режим за замовчуванням: ANSI / ASCII-подібний (multibyte)\n"));
    else if (sizeof(TCHAR) == sizeof(wchar_t))
        _tprintf(_T("Режим за замовчуванням: UNICODE\n"));
    else
        _tprintf(_T("Невідомий режим.\n"));
}

void printWideTextToConsole(const wstring& text)
{
    fflush(stdout); // Відновлено очищення буфера
    int oldMode = _setmode(_fileno(stdout), _O_U16TEXT);
    wcout << text << endl;
    wcout.flush();  // Відновлено очищення буфера
    _setmode(_fileno(stdout), oldMode);

    SetConsoleOutputCP(1251);
}

void printMacroInfo()
{
    printSeparator(_T("2. Перевірка макросів командного рядка"));

    #ifdef UNICODE
        _tprintf(_T("Макрос UNICODE: ВИЗНАЧЕНО\n"));
    #else
        _tprintf(_T("Макрос UNICODE: НЕ ВИЗНАЧЕНО\n"));
    #endif

    #ifdef _UNICODE
        _tprintf(_T("Макрос _UNICODE: ВИЗНАЧЕНО\n"));
    #else
        _tprintf(_T("Макрос _UNICODE: НЕ ВИЗНАЧЕНО\n"));
    #endif
}


// ======================================================
// Основні функції
// ======================================================

// Перетворення ANSI -> Unicode
wstring ansiToWide(const string& ansi)
{
    if (ansi.empty())
        return L"";

    int requiredSize = MultiByteToWideChar(1251, 0, ansi.c_str(), -1, NULL, 0);

    if (requiredSize == 0)
        return L"";

    vector<wchar_t> buffer(requiredSize);

    MultiByteToWideChar(1251, 0, ansi.c_str(), -1, buffer.data(), requiredSize);

    return wstring(buffer.data());
}

// Перетворення Unicode -> ANSI
string wideToAnsi(const wstring& wide)
{
    if (wide.empty())
        return "";

    BOOL usedDefaultChar = FALSE;

    int requiredSize = WideCharToMultiByte(1251, 0, wide.c_str(), -1, NULL, 0, NULL, &usedDefaultChar);

    if (requiredSize == 0)
        return "";

    vector<char> buffer(requiredSize);

    WideCharToMultiByte(1251, 0, wide.c_str(), -1, buffer.data(), requiredSize, NULL, &usedDefaultChar);

    return string(buffer.data());
}

// Виведення Unicode через wcout
void printWideWithWcout(const vector<wstring>& arr)
{
    printSeparator(_T("Виведення через wcout"));

    fflush(stdout); // Відновлено очищення буфера
    int oldMode = _setmode(_fileno(stdout), _O_U16TEXT);

    for (size_t i = 0; i < arr.size(); i++)
    {
        wcout << L"[" << i + 1 << L"] " << arr[i] << endl;
    }

    wcout.flush(); // Відновлено очищення буфера
    _setmode(_fileno(stdout), oldMode);
}

// Виведення Unicode через _tprintf
void printWideWithTprintf(const vector<wstring>& arr)
{
    printSeparator(_T("Виведення через _tprintf"));

    fflush(stdout);

    #ifdef UNICODE
        int oldMode = _setmode(_fileno(stdout), _O_U16TEXT); // включаємо UTF-16
    #endif

    for (size_t i = 0; i < arr.size(); i++)
    {
        _tprintf(_T("[%d] %ls\n"), (int)(i + 1), arr[i].c_str());
    }

    #ifdef UNICODE
        fflush(stdout); 
        _setmode(_fileno(stdout), oldMode); // повертаємо старий режим
    #endif
}

// Виведення Unicode через MessageBox
void printWideWithMessageBox(const vector<wstring>& arr)
{
    printSeparator(_T("Виведення через MessageBox"));

    wstring allText;
    for (size_t i = 0; i < arr.size(); i++)
    {
        allText += L"[";
        allText += to_wstring(i + 1);
        allText += L"] ";
        allText += arr[i];
        allText += L"\n";
    }

    MessageBoxW(NULL, allText.c_str(), L"Unicode-масив", MB_OK);
}


// ======================================================
// Функції сортування
// ======================================================

int __cdecl compareWideStrings(const void* a, const void* b)
{
    const wchar_t* const* s1 = (const wchar_t* const*)a;
    const wchar_t* const* s2 = (const wchar_t* const*)b;
    return wcscmp(*s1, *s2);
}

vector<wstring> qsort(const vector<wstring>& source)
{
    vector<wchar_t*> ptrs;
    ptrs.reserve(source.size());

    for (const auto& s : source)
    {
        size_t len = s.length() + 1;
        wchar_t* temp = new wchar_t[len];
        wcscpy_s(temp, len, s.c_str());
        ptrs.push_back(temp);
    }

    qsort(ptrs.data(), ptrs.size(), sizeof(wchar_t*), compareWideStrings);

    vector<wstring> result;
    result.reserve(ptrs.size());

    for (size_t i = 0; i < ptrs.size(); i++)
    {
        result.push_back(ptrs[i]);
        delete[] ptrs[i];
    }

    return result;
}

vector<wstring> stdsort(vector<wstring> source)
{
    sort(source.begin(), source.end());
    return source;
}


// ======================================================
// Додаткове завдання 
// ======================================================

bool readFileBytes(const string& fileName, vector<char>& data)
{
    ifstream fin(fileName, ios::binary);
    if (!fin.is_open())
        return false;

    fin.seekg(0, ios::end);
    streamsize size = fin.tellg();
    fin.seekg(0, ios::beg);

    if (size < 0)
    {
        fin.close();
        return false;
    }

    data.resize((size_t)size);

    if (size > 0)
        fin.read(data.data(), size);

    fin.close();
    return true;
}


// Визначення типу кодування файла
FileEncoding detectEncoding(const vector<char>& data)
{
    // Перевірка BOM для UTF-16 LE: FF FE
    if (data.size() >= 2)
    {
        unsigned char b0 = (unsigned char)data[0];
        unsigned char b1 = (unsigned char)data[1];

        if (b0 == 0xFF && b1 == 0xFE)
            return FileEncoding::UNICODE_UTF16_LE;
    }

    // Додаткова перевірка на Unicode за допомогою Windows API
    int test = -1;
    BOOL isUnicode = IsTextUnicode(data.data(), (int)data.size(), &test);

    if (isUnicode)
        return FileEncoding::UNICODE_UTF16_LE;

    // У протилежному випадку вважаємо файл ANSI
    return FileEncoding::ANSI_TEXT;
}


// Перетворення вмісту файла у wstring
wstring bytesToWideText(const vector<char>& data, FileEncoding enc)
{
    if (data.empty())
        return L"";

    if (enc == FileEncoding::UNICODE_UTF16_LE)
    {
        size_t start = 0;

        // Якщо у файлі є BOM, пропускаємо його
        if (data.size() >= 2 &&
            (unsigned char)data[0] == 0xFF &&
            (unsigned char)data[1] == 0xFE)
        {
            start = 2;
        }

        size_t wcharCount = (data.size() - start) / sizeof(wchar_t);
        const wchar_t* wptr = reinterpret_cast<const wchar_t*>(data.data() + start);

        return wstring(wptr, wptr + wcharCount);
    }
    else
    {
        // Якщо файл ANSI, перетворюємо його в Unicode
        string ansi(data.begin(), data.end());
        return ansiToWide(ansi);
    }
}


// Збереження результату у файл
bool saveWideText(const string& fileName, const wstring& text, FileEncoding enc)
{
    ofstream fout(fileName, ios::binary);
    if (!fout.is_open())
        return false;

    if (enc == FileEncoding::UNICODE_UTF16_LE)
    {
        // Записуємо BOM UTF-16 LE
        unsigned char bom[2] = { 0xFF, 0xFE };
        fout.write((const char*)bom, 2);

        // Записуємо сам текст у wide-форматі
        fout.write((const char*)text.c_str(), text.size() * sizeof(wchar_t));
    }
    else
    {
        // Для ANSI виконуємо зворотне перетворення з Unicode
        string ansi = wideToAnsi(text);
        fout.write(ansi.c_str(), ansi.size());
    }

    fout.close();
    return true;
}


void printReversedFileTextToConsole(const wstring& text, FileEncoding enc)
{
    fflush(stdout); // Відновлено очищення буфера

    if (enc == FileEncoding::ANSI_TEXT)
    {
        SetConsoleOutputCP(1251);
        _setmode(_fileno(stdout), _O_TEXT);

        string ansiText = wideToAnsi(text);
        printf("%s\n", ansiText.c_str());

        fflush(stdout); // Відновлено очищення буфера
    }
    else
    {
        int oldMode = _setmode(_fileno(stdout), _O_U16TEXT);
        wcout << text << endl;

        wcout.flush(); // Відновлено очищення буфера
        _setmode(_fileno(stdout), oldMode);

        SetConsoleOutputCP(1251);
    }
}


void reverseTextFilePreserveEncoding(const string& inputFile, const string& outputFile)
{
    printSeparator(_T("Додаткове завдання: файл у зворотному порядку"));

    vector<char> bytes;

    if (!readFileBytes(inputFile, bytes))
    {
        _tprintf(_T("Не вдалося відкрити вхідний файл: %S\n"), inputFile.c_str());
        return;
    }

    FileEncoding enc = detectEncoding(bytes);

    if (enc == FileEncoding::UNICODE_UTF16_LE)
        _tprintf(_T("Визначено тип файлу: UNICODE UTF-16 LE\n"));
    else
        _tprintf(_T("Визначено тип файлу: ANSI / ASCII-подібний\n"));

    wstring text = bytesToWideText(bytes, enc);

    reverse(text.begin(), text.end());

    _tprintf(_T("Вміст тексту після перестановки символів у зворотному порядку:\n"));
    printReversedFileTextToConsole(text, enc);

    if (saveWideText(outputFile, text, enc))
        _tprintf(_T("Результат збережено у файл: %S\n"), outputFile.c_str());
    else
        _tprintf(_T("Не вдалося зберегти результат.\n"));
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

int _tmain()
{
    const string input_ansi_file = "input_ansi.txt";
    const string output_ansi_file = "output_ansi_reversed.txt";
    const string input_unicode_file = "input_unicode.txt";
    const string output_unicode_file = "output_unicode_reversed.txt";

    setupLocale();

    printEncodingInfo();
    printMacroInfo();

    printSeparator(_T("4. Формування ANSI-масиву з ПІБ"));

    vector<wstring> familyWideSource = {
        L"Кулик Євген Вадимович",
        L"Клочков Ілля Віталійович",
        L"Калашник Андрій Миколайович"
    };

    vector<string> familyAnsi;
    for (const auto& ws : familyWideSource)
    {
        familyAnsi.push_back(wideToAnsi(ws));
    }

    _tprintf(_T("ANSI-рядки:\n"));
    fflush(stdout);
    SetConsoleOutputCP(1251);
    for (size_t i = 0; i < familyAnsi.size(); i++)
    {
        printf("[%d] %s\n", (int)(i + 1), familyAnsi[i].c_str());
    }

    printSeparator(_T("5. Перетворення ANSI -> UNICODE"));

    vector<wstring> familyUnicode;
    for (const auto& s : familyAnsi)
    {
        familyUnicode.push_back(ansiToWide(s));
    }

    _tprintf(_T("Масив Unicode сформовано успішно.\n"));

    printWideWithTprintf(familyUnicode);
    printWideWithWcout(familyUnicode);
    printWideWithMessageBox(familyUnicode);

    printSeparator(_T("6. Сортування Unicode-масиву через qsort"));

    vector<wstring> qsorted = qsort(familyUnicode);
    printWideWithTprintf(qsorted);

    printSeparator(_T("7. Сортування Unicode-масиву через std::sort"));

    vector<wstring> stdsorted = stdsort(familyUnicode);
    printWideWithTprintf(stdsorted);

    printSeparator(_T("8. Зворотне перетворення UNICODE -> ANSI"));

    vector<string> backToAnsi;
    for (const auto& ws : stdsorted)
    {
        backToAnsi.push_back(wideToAnsi(ws));
    }

    _tprintf(_T("Результат після зворотного перетворення:\n"));
    fflush(stdout);           
    SetConsoleOutputCP(1251); 
    for (size_t i = 0; i < backToAnsi.size(); i++)
    {
        printf("[%d] %s\n", (int)(i + 1), backToAnsi[i].c_str());
    }

    // --------------------------------------------------
    // Додаткове завдання на найвищу оцінку
    // --------------------------------------------------

    reverseTextFilePreserveEncoding(input_ansi_file, output_ansi_file);
    reverseTextFilePreserveEncoding(input_unicode_file, output_unicode_file);

    return 0;
}