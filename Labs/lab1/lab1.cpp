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

// ======================================================
// Допоміжні функції
// ======================================================

void PrintSeparator(const TCHAR* title)
{
    _tprintf(TEXT("\n==================================================\n"));
    _tprintf(TEXT("%s\n"), title);
    _tprintf(TEXT("==================================================\n"));
}

void SetupLocale()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (_tsetlocale(LC_ALL, TEXT("Ukrainian")) == NULL)
    {
        _tprintf(TEXT("Не вдалося встановити локаль Ukrainian.\n"));
    }

    setlocale(LC_ALL, "Ukrainian");
}

void PrintEncodingInfo()
{
    PrintSeparator(TEXT("1. Перевірка типу кодування за замовчуванням"));

    _tprintf(TEXT("sizeof(TCHAR) = %d байт(и)\n"), (int)sizeof(TCHAR));

    if (sizeof(TCHAR) == sizeof(char))
        _tprintf(TEXT("Режим за замовчуванням: ANSI / ASCII-подібний (multibyte)\n"));
    else if (sizeof(TCHAR) == sizeof(wchar_t))
        _tprintf(TEXT("Режим за замовчуванням: UNICODE\n"));
    else
        _tprintf(TEXT("Невідомий режим.\n"));
}

void PrintWideTextToConsole(const wstring& text)
{
    int oldMode = _setmode(_fileno(stdout), _O_U16TEXT);
    wcout << text << endl;
    _setmode(_fileno(stdout), oldMode);

    SetConsoleOutputCP(1251);
}

void PrintMacroInfo()
{
    PrintSeparator(TEXT("2. Перевірка макросів командного рядка"));

#ifdef UNICODE
    _tprintf(TEXT("Макрос UNICODE: ВИЗНАЧЕНО\n"));
#else
    _tprintf(TEXT("Макрос UNICODE: НЕ ВИЗНАЧЕНО\n"));
#endif

#ifdef _UNICODE
    _tprintf(TEXT("Макрос _UNICODE: ВИЗНАЧЕНО\n"));
#else
    _tprintf(TEXT("Макрос _UNICODE: НЕ ВИЗНАЧЕНО\n"));
#endif
}

void ShowProjectModeReminder()
{
    PrintSeparator(TEXT("3. Нагадування щодо перемикання режиму в Visual Studio"));
    _tprintf(TEXT("ASCII (Multibyte): Properties -> General -> Character Set -> Use Multi-Byte Character Set\n"));
    _tprintf(TEXT("UNICODE:          Properties -> General -> Character Set -> Use Unicode Character Set\n"));
    _tprintf(TEXT("Після перемикання треба знову зібрати і запустити програму.\n"));
}

// ------------------------------------------------------
// Перетворення ANSI -> Unicode
// ------------------------------------------------------

wstring AnsiToWide(const string& ansi)
{
    if (ansi.empty())
        return L"";

    int requiredSize = MultiByteToWideChar(
        1251,
        0,
        ansi.c_str(),
        -1,
        NULL,
        0
    );

    if (requiredSize == 0)
        return L"";

    vector<wchar_t> buffer(requiredSize);

    MultiByteToWideChar(
        1251,
        0,
        ansi.c_str(),
        -1,
        buffer.data(),
        requiredSize
    );

    return wstring(buffer.data());
}

// ------------------------------------------------------
// Перетворення Unicode -> ANSI
// ------------------------------------------------------

string WideToAnsi(const wstring& wide)
{
    if (wide.empty())
        return "";

    BOOL usedDefaultChar = FALSE;

    int requiredSize = WideCharToMultiByte(
        1251,
        0,
        wide.c_str(),
        -1,
        NULL,
        0,
        NULL,
        &usedDefaultChar
    );

    if (requiredSize == 0)
        return "";

    vector<char> buffer(requiredSize);

    WideCharToMultiByte(
        1251,
        0,
        wide.c_str(),
        -1,
        buffer.data(),
        requiredSize,
        NULL,
        &usedDefaultChar
    );

    return string(buffer.data());
}

// ------------------------------------------------------
// Виведення Unicode через wcout
// ------------------------------------------------------

void PrintWideWithWcout(const vector<wstring>& arr)
{
    PrintSeparator(TEXT("Виведення через wcout"));

    int oldMode = _setmode(_fileno(stdout), _O_U16TEXT);

    for (size_t i = 0; i < arr.size(); i++)
    {
        wcout << L"[" << i + 1 << L"] " << arr[i] << endl;
    }

    _setmode(_fileno(stdout), oldMode);
}

// ------------------------------------------------------
// Виведення Unicode через _tprintf
// ------------------------------------------------------

void PrintWideWithTprintf(const vector<wstring>& arr)
{
    PrintSeparator(TEXT("Виведення через _tprintf"));

#ifdef UNICODE
    int oldMode = _setmode(_fileno(stdout), _O_U16TEXT);
#endif

    for (size_t i = 0; i < arr.size(); i++)
    {
        _tprintf(TEXT("[%d] %ls\n"), (int)(i + 1), arr[i].c_str());
    }

#ifdef UNICODE
    _setmode(_fileno(stdout), oldMode);
#endif
}

// ------------------------------------------------------
// Виведення Unicode через MessageBox
// ------------------------------------------------------

void PrintWideWithMessageBox(const vector<wstring>& arr)
{
    PrintSeparator(TEXT("Виведення через MessageBox"));

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
// qsort для Unicode-масиву
// ======================================================

int __cdecl CompareWideStrings(const void* a, const void* b)
{
    const wchar_t* const* s1 = (const wchar_t* const*)a;
    const wchar_t* const* s2 = (const wchar_t* const*)b;
    return wcscmp(*s1, *s2);
}

vector<wstring> SortWithQsort(const vector<wstring>& source)
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

    qsort(ptrs.data(), ptrs.size(), sizeof(wchar_t*), CompareWideStrings);

    vector<wstring> result;
    result.reserve(ptrs.size());

    for (size_t i = 0; i < ptrs.size(); i++)
    {
        result.push_back(ptrs[i]);
        delete[] ptrs[i];
    }

    return result;
}

vector<wstring> SortWithStdSort(vector<wstring> source)
{
    sort(source.begin(), source.end());
    return source;
}

// ======================================================
// Додаткове завдання для найвищої оцінки
// Незалежно від способу кодування символів у текстовому
// файлі переставити символи у зворотному порядку
// ======================================================

// У межах даної лабораторної роботи будемо підтримувати
// два основних варіанти кодування:
// 1) ANSI / ASCII-подібне кодування
// 2) Unicode UTF-16 LE
enum class FileEncoding
{
    ANSI_TEXT,
    UNICODE_UTF16_LE
};

// ------------------------------------------------------
// Зчитування всіх байтів файла у вектор<char>
// ------------------------------------------------------
bool ReadFileBytes(const string& fileName, vector<char>& data)
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

// ------------------------------------------------------
// Визначення типу кодування файла
// ------------------------------------------------------
// Спочатку перевіряємо наявність BOM UTF-16 LE.
// Якщо BOM немає, додатково використовуємо IsTextUnicode.
// Якщо Unicode не підтвердився, вважаємо файл ANSI.
FileEncoding DetectEncoding(const vector<char>& data)
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

// ------------------------------------------------------
// Перетворення вмісту файла у wstring
// ------------------------------------------------------
// Це потрібно для того, щоб далі однаково зручно
// працювати і з ANSI, і з Unicode-текстом.
wstring BytesToWideText(const vector<char>& data, FileEncoding enc)
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
        // за допомогою вже створеної функції AnsiToWide
        string ansi(data.begin(), data.end());
        return AnsiToWide(ansi);
    }
}

// ------------------------------------------------------
// Збереження результату у файл
// ------------------------------------------------------
// Якщо вхідний файл був Unicode, то і результат зберігаємо
// як Unicode UTF-16 LE.
// Якщо вхідний файл був ANSI, то і результат зберігаємо як ANSI.
bool SaveWideText(const string& fileName, const wstring& text, FileEncoding enc)
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
        string ansi = WideToAnsi(text);
        fout.write(ansi.c_str(), ansi.size());
    }

    fout.close();
    return true;
}

// ------------------------------------------------------
// Допоміжне виведення результату в консоль
// ------------------------------------------------------
// Для ANSI-файла виводимо через printf у кодовій сторінці 1251.
// Для Unicode-файла використовуємо wide-виведення через wcout.
void PrintReversedFileTextToConsole(const wstring& text, FileEncoding enc)
{
    if (enc == FileEncoding::ANSI_TEXT)
    {
        SetConsoleOutputCP(1251);
        _setmode(_fileno(stdout), _O_TEXT);

        string ansiText = WideToAnsi(text);
        printf("%s\n", ansiText.c_str());
    }
    else
    {
        int oldMode = _setmode(_fileno(stdout), _O_U16TEXT);
        wcout << text << endl;
        _setmode(_fileno(stdout), oldMode);

        SetConsoleOutputCP(1251);
    }
}

// ------------------------------------------------------
// Основна функція додаткового завдання
// ------------------------------------------------------
// 1. Зчитуємо файл
// 2. Визначаємо кодування
// 3. Перетворюємо текст у wstring
// 4. Переставляємо символи у зворотному порядку
// 5. Виводимо результат у консоль
// 6. Зберігаємо результат у новий файл
void ReverseTextFilePreserveEncoding(const string& inputFile, const string& outputFile)
{
    PrintSeparator(TEXT("Додаткове завдання: файл у зворотному порядку"));

    vector<char> bytes;

    // Перевіряємо, чи вдалося відкрити файл
    if (!ReadFileBytes(inputFile, bytes))
    {
        _tprintf(TEXT("Не вдалося відкрити вхідний файл: %S\n"), inputFile.c_str());
        return;
    }

    // Визначаємо тип кодування файла
    FileEncoding enc = DetectEncoding(bytes);

    if (enc == FileEncoding::UNICODE_UTF16_LE)
        _tprintf(TEXT("Визначено тип файлу: UNICODE UTF-16 LE\n"));
    else
        _tprintf(TEXT("Визначено тип файлу: ANSI / ASCII-подібний\n"));

    // Перетворюємо байти файла у Unicode-рядок
    wstring text = BytesToWideText(bytes, enc);

    // Переставляємо символи у зворотному порядку
    reverse(text.begin(), text.end());

    // Виводимо результат у консоль
    _tprintf(TEXT("Вміст тексту після перестановки символів у зворотному порядку:\n"));
    PrintReversedFileTextToConsole(text, enc);

    // Зберігаємо перевернутий текст у новий файл
    if (SaveWideText(outputFile, text, enc))
        _tprintf(TEXT("Результат збережено у файл: %S\n"), outputFile.c_str());
    else
        _tprintf(TEXT("Не вдалося зберегти результат.\n"));
}

// ======================================================
// Основна лабораторна логіка
// ======================================================

void task1_Lab1()
{
    SetupLocale();

    PrintEncodingInfo();
    PrintMacroInfo();
    ShowProjectModeReminder();

    PrintSeparator(TEXT("4. Формування ANSI-масиву з ПІБ"));

    // Для надійності джерельного файла спочатку задаємо wide-рядки,
    // а потім будуємо ANSI-подання програмно.
    vector<wstring> familyWideSource =
    {
        L"Кулик Євген Вадимович",
        L"Клочков Ілля Віталійович",
        L"Калашник Андрій Миколайович"
    };

    vector<string> familyAnsi;
    for (const auto& ws : familyWideSource)
    {
        familyAnsi.push_back(WideToAnsi(ws));
    }

    _tprintf(TEXT("ANSI-рядки:\n"));
    for (size_t i = 0; i < familyAnsi.size(); i++)
    {
        printf("[%d] %s\n", (int)(i + 1), familyAnsi[i].c_str());
    }

    PrintSeparator(TEXT("5. Перетворення ANSI -> UNICODE"));

    vector<wstring> familyUnicode;
    for (const auto& s : familyAnsi)
    {
        familyUnicode.push_back(AnsiToWide(s));
    }

    _tprintf(TEXT("Масив Unicode сформовано успішно.\n"));

    PrintWideWithTprintf(familyUnicode);
    PrintWideWithWcout(familyUnicode);
    PrintWideWithMessageBox(familyUnicode);

    PrintSeparator(TEXT("6. Сортування Unicode-масиву через qsort"));

    vector<wstring> qsorted = SortWithQsort(familyUnicode);
    PrintWideWithTprintf(qsorted);

    PrintSeparator(TEXT("7. Сортування Unicode-масиву через std::sort"));

    vector<wstring> stdsorted = SortWithStdSort(familyUnicode);
    PrintWideWithTprintf(stdsorted);

    PrintSeparator(TEXT("8. Зворотне перетворення UNICODE -> ANSI"));

    vector<string> backToAnsi;
    for (const auto& ws : stdsorted)
    {
        backToAnsi.push_back(WideToAnsi(ws));
    }

    _tprintf(TEXT("Результат після зворотного перетворення:\n"));
    for (size_t i = 0; i < backToAnsi.size(); i++)
    {
        printf("[%d] %s\n", (int)(i + 1), backToAnsi[i].c_str());
    }

    // --------------------------------------------------
    // Додаткове завдання на найвищу оцінку
    // --------------------------------------------------

    ReverseTextFilePreserveEncoding("input_ansi.txt", "output_ansi_reversed.txt");
    ReverseTextFilePreserveEncoding("input_unicode.txt", "output_unicode_reversed.txt");
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
    task1_Lab1();
    return 0;
}