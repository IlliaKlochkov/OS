#include <windows.h>
#include <tchar.h>
#include <locale.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// ======================================================
// Константи
// ======================================================

static const TCHAR* MAILBOX_DIR = _T("Mailbox");
static const TCHAR* META_FILE = _T("Mailbox\\mailbox_meta.bin");
static const TCHAR* MESSAGES_DIR = _T("Mailbox\\messages");
static int          g_mailboxCount = 0;

// ======================================================
// Структура заголовку поштової скриньки
// ======================================================

#pragma pack(push, 1)
struct MailboxHeader
{
    DWORD msgCount;     // кількість повідомлень
    DWORD totalSize;    // загальний розмір усіх повідомлень (байт)
    DWORD maxSize;      // максимальний розмір скриньки (байт)
    DWORD crc32;        // CRC32 цього заголовку (перші 12 байт)
};
#pragma pack(pop)

// ======================================================
// CRC32
// ======================================================

static DWORD crc32Table[256];
static bool  crc32TableReady = false;

void buildCRC32Table()
{
    if (crc32TableReady) return;
    for (DWORD i = 0; i < 256; i++)
    {
        DWORD c = i;
        for (int j = 0; j < 8; j++)
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        crc32Table[i] = c;
    }
    crc32TableReady = true;
}

DWORD calcCRC32(const BYTE* data, DWORD len)
{
    buildCRC32Table();
    DWORD crc = 0xFFFFFFFFu;
    for (DWORD i = 0; i < len; i++)
        crc = crc32Table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

// ======================================================
// Допоміжні функції
// ======================================================

void setupLocale()
{
    setlocale(LC_ALL, "Ukrainian");
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
}

void printSeparator(const char* title)
{
    printf("\n==================================================\n");
    printf("%s\n", title);
    printf("==================================================\n");
}

// Формуємо шлях до файлу повідомлення: messages\msg_NNNN.bin
string getMessagePath(DWORD index)
{
    ostringstream oss;
    oss << "Mailbox\\messages\\msg_" << setw(4) << setfill('0') << index << ".bin";
    return oss.str();
}

// ======================================================
// Операції з мета-файлом (заголовок скриньки)
// ======================================================

bool readHeader(MailboxHeader& hdr)
{
    HANDLE hFile = CreateFileA(
        "Mailbox\\mailbox_meta.bin",
        GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    DWORD bytesRead = 0;
    BOOL ok = ReadFile(hFile, &hdr, sizeof(hdr), &bytesRead, NULL);
    CloseHandle(hFile);

    return ok && (bytesRead == sizeof(hdr));
}

bool writeHeader(const MailboxHeader& hdr)
{
    HANDLE hFile = CreateFileA(
        "Mailbox\\mailbox_meta.bin",
        GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    DWORD bytesWritten = 0;
    BOOL ok = WriteFile(hFile, &hdr, sizeof(hdr), &bytesWritten, NULL);
    CloseHandle(hFile);

    return ok && (bytesWritten == sizeof(hdr));
}

// CRC32 рахується по перших 12 байтах заголовка (без поля crc32)
DWORD calcHeaderCRC(const MailboxHeader& hdr)
{
    return calcCRC32((const BYTE*)&hdr, 12);
}

// ======================================================
// Ініціалізація / Створення поштової скриньки
// ======================================================

bool createMailbox(DWORD maxSizeBytes)
{
    // Створюємо каталог Mailbox
    if (!CreateDirectoryA("Mailbox", NULL) &&
        GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Помилка: неможливо створити каталог Mailbox\n");
        return false;
    }

    // Створюємо підкаталог Mailbox\messages
    if (!CreateDirectoryA("Mailbox\\messages", NULL) &&
        GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Помилка: неможливо створити каталог messages\n");
        return false;
    }

    MailboxHeader hdr = {};
    hdr.msgCount = 0;
    hdr.totalSize = 0;
    hdr.maxSize = maxSizeBytes;
    hdr.crc32 = calcHeaderCRC(hdr);

    if (!writeHeader(hdr))
    {
        printf("Помилка: неможливо записати мета-файл\n");
        return false;
    }

    g_mailboxCount++;
    printf("Поштову скриньку створено. Максимальний розмір: %u байт\n", maxSizeBytes);
    return true;
}

// ======================================================
// Додавання листа
// ======================================================

bool addMessage(const string& body)
{
    MailboxHeader hdr;
    if (!readHeader(hdr))
    {
        printf("Помилка читання заголовку скриньки\n");
        return false;
    }

    DWORD msgSize = (DWORD)body.size();

    if (hdr.totalSize + msgSize > hdr.maxSize)
    {
        printf("Помилка: поштова скринька переповнена! (%u + %u > %u)\n",
            hdr.totalSize, msgSize, hdr.maxSize);
        return false;
    }

    // Знаходимо наступний вільний індекс (може бути «дірка» через видалення)
    DWORD newIndex = hdr.msgCount;

    string msgPath = getMessagePath(newIndex);

    HANDLE hFile = CreateFileA(
        msgPath.c_str(),
        GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Помилка: неможливо створити файл повідомлення\n");
        return false;
    }

    // Спочатку записуємо розмір повідомлення (4 байти)
    DWORD bytesWritten = 0;
    WriteFile(hFile, &msgSize, sizeof(msgSize), &bytesWritten, NULL);

    // Потім саме тіло
    WriteFile(hFile, body.c_str(), msgSize, &bytesWritten, NULL);
    CloseHandle(hFile);

    hdr.msgCount++;
    hdr.totalSize += msgSize;
    hdr.crc32 = calcHeaderCRC(hdr);

    if (!writeHeader(hdr))
    {
        printf("Помилка оновлення заголовку\n");
        return false;
    }

    printf("Лист #%u додано (%u байт)\n", newIndex, msgSize);
    return true;
}

// ======================================================
// Читання листа (з видаленням або без)
// ======================================================

bool readMessage(DWORD index, bool deleteAfterRead)
{
    string msgPath = getMessagePath(index);

    HANDLE hFile = CreateFileA(
        msgPath.c_str(),
        deleteAfterRead ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
        0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Помилка: лист #%u не знайдено\n", index);
        return false;
    }

    DWORD msgSize = 0, bytesRead = 0;
    ReadFile(hFile, &msgSize, sizeof(msgSize), &bytesRead, NULL);

    string body(msgSize, '\0');
    ReadFile(hFile, &body[0], msgSize, &bytesRead, NULL);
    CloseHandle(hFile);

    printf("--- Лист #%u (%u байт) ---\n%s\n", index, msgSize, body.c_str());

    if (deleteAfterRead)
    {
        if (!DeleteFileA(msgPath.c_str()))
        {
            printf("Попередження: не вдалось видалити файл листа\n");
            return false;
        }

        MailboxHeader hdr;
        if (readHeader(hdr))
        {
            if (hdr.msgCount > 0) hdr.msgCount--;
            if (hdr.totalSize >= msgSize) hdr.totalSize -= msgSize;
            hdr.crc32 = calcHeaderCRC(hdr);
            writeHeader(hdr);
        }
        printf("(Лист #%u видалено після читання)\n", index);
    }

    return true;
}

// ======================================================
// Видалення конкретного листа
// ======================================================

bool deleteMessage(DWORD index)
{
    string msgPath = getMessagePath(index);

    // Зчитаємо розмір перед видаленням
    HANDLE hFile = CreateFileA(
        msgPath.c_str(),
        GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD msgSize = 0;
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD br = 0;
        ReadFile(hFile, &msgSize, sizeof(msgSize), &br, NULL);
        CloseHandle(hFile);
    }
    else
    {
        printf("Помилка: лист #%u не знайдено\n", index);
        return false;
    }

    if (!DeleteFileA(msgPath.c_str()))
    {
        printf("Помилка видалення листа #%u\n", index);
        return false;
    }

    MailboxHeader hdr;
    if (readHeader(hdr))
    {
        if (hdr.msgCount > 0)  hdr.msgCount--;
        if (hdr.totalSize >= msgSize) hdr.totalSize -= msgSize;
        hdr.crc32 = calcHeaderCRC(hdr);
        writeHeader(hdr);
    }

    printf("Лист #%u видалено\n", index);
    return true;
}

// ======================================================
// Видалення всіх листів
// ======================================================

bool deleteAllMessages()
{
    MailboxHeader hdr;
    if (!readHeader(hdr)) return false;

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("Mailbox\\messages\\msg_*.bin", &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("Листів немає — скринька вже порожня\n");
        return true;
    }

    DWORD deleted = 0;
    do
    {
        string fullPath = string("Mailbox\\messages\\") + fd.cFileName;
        if (DeleteFileA(fullPath.c_str()))
            deleted++;
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);

    hdr.msgCount = 0;
    hdr.totalSize = 0;
    hdr.crc32 = calcHeaderCRC(hdr);
    writeHeader(hdr);

    printf("Видалено %u листів\n", deleted);
    return true;
}

// ======================================================
// Кількість листів
// ======================================================

DWORD getMessageCount()
{
    MailboxHeader hdr;
    if (!readHeader(hdr)) return 0;
    return hdr.msgCount;
}

// ======================================================
// Загальна кількість поштових скриньок
// ======================================================

int getTotalMailboxCount()
{
    return g_mailboxCount;
}

// ======================================================
// Контроль цілісності (CRC32)
// ======================================================

bool checkIntegrity()
{
    MailboxHeader hdr;
    if (!readHeader(hdr))
    {
        printf("Помилка: не вдалось прочитати заголовок для перевірки цілісності\n");
        return false;
    }

    DWORD expected = calcHeaderCRC(hdr);
    if (expected != hdr.crc32)
    {
        printf("ЦІЛІСНІСТЬ ПОРУШЕНА! CRC заголовку: збережений=0x%08X, розрахований=0x%08X\n",
            hdr.crc32, expected);
        return false;
    }

    // Перевіряємо реальну кількість файлів
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("Mailbox\\messages\\msg_*.bin", &fd);
    DWORD realCount = 0;
    DWORD realSize = 0;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            string fullPath = string("Mailbox\\messages\\") + fd.cFileName;
            HANDLE hMsg = CreateFileA(
                fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (hMsg != INVALID_HANDLE_VALUE)
            {
                DWORD msgSize = 0, br = 0;
                ReadFile(hMsg, &msgSize, sizeof(msgSize), &br, NULL);
                CloseHandle(hMsg);

                realCount++;
                realSize += msgSize;
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }

    bool ok = (realCount == hdr.msgCount) && (realSize == hdr.totalSize);

    if (ok)
    {
        printf("Цілісність OK: CRC=0x%08X, листів=%u, розмір=%u байт\n",
            hdr.crc32, hdr.msgCount, hdr.totalSize);
    }
    else
    {
        printf("НЕВІДПОВІДНІСТЬ! Заголовок каже: листів=%u, розмір=%u; "
            "Реально: листів=%u, розмір=%u\n",
            hdr.msgCount, hdr.totalSize, realCount, realSize);
    }

    return ok;
}

// ======================================================
// Виведення інформації про скриньку
// ======================================================

void printMailboxInfo()
{
    MailboxHeader hdr;
    if (!readHeader(hdr))
    {
        printf("Помилка читання заголовку\n");
        return;
    }

    printf("  Кількість листів : %u\n", hdr.msgCount);
    printf("  Загальний розмір : %u байт\n", hdr.totalSize);
    printf("  Максимальний розмір: %u байт\n", hdr.maxSize);
    printf("  CRC32 заголовку : 0x%08X\n", hdr.crc32);
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


int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printSeparator("Лабораторна робота #4: Моделювання поштової скриньки (WinAPI)");

    // крок 1: Створення скриньки 
    printSeparator("1. Створення поштової скриньки (макс. 4096 байт)");
    if (!createMailbox(4096))
        return 1;

    printf("Загальна кількість поштових скриньок: %d\n", getTotalMailboxCount());

    // крок 2: Додавання листів
    printSeparator("2. Додавання листів");
    addMessage("Привіт! Це перший лист у поштовій скриньці.");
    addMessage("Другий лист: WinAPI дозволяє працювати з файлами напряму.");
    addMessage("Третій лист: CreateFile, ReadFile, WriteFile, DeleteFile.");
    addMessage("Четвертий лист: Короткий.");

    // крок 3: Інформація
    printSeparator("3. Стан скриньки після додавання");
    printMailboxInfo();

    // крок 4: Читання без видалення
    printSeparator("4. Читання листа #0 (без видалення)");
    readMessage(0, false);

    // крок 5: Читання з видаленням
    printSeparator("5. Читання листа #1 (з видаленням)");
    readMessage(1, true);

    // крок 6: Стан після
    printSeparator("6. Стан скриньки після читання з видаленням");
    printMailboxInfo();

    // крок 7: Видалення конкретного листа
    printSeparator("7. Видалення листа #2");
    deleteMessage(2);
    printf("Кількість листів: %u\n", getMessageCount());

    // крок 8: Перевірка цілісності
    printSeparator("8. Перевірка цілісності (CRC32)");
    checkIntegrity();

    // крок 9: Видалення всіх листів
    printSeparator("9. Видалення всіх листів");
    deleteAllMessages();
    printf("Кількість листів: %u\n", getMessageCount());

    // крок 10: Фінальна перевірка цілісності -
    printSeparator("10. Фінальна перевірка цілісності");
    checkIntegrity();

    printf("\nГотово.\n");
    return 0;
}