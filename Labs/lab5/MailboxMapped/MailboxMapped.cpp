#include <windows.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// ======================================================
// Константи шляхів
// ======================================================

static const char* MAILBOX_META = "Mailbox\\mailbox_meta.bin";

// ======================================================
// Структура заголовку скриньки
// ======================================================

#pragma pack(push, 1)
struct MailboxHeader
{
    DWORD msgCount;   // кількість повідомлень
    DWORD totalSize;  // загальний розмір тіл листів (байт)
    DWORD maxSize;    // максимальний ліміт скриньки
    DWORD crc32;      // CRC32 перших 12 байт заголовку
};
#pragma pack(pop)

// ======================================================
// CRC32
// ======================================================

static DWORD g_crcTable[256];
static bool  g_crcReady = false;

// Таблиця для швидкого обчислення CRC32 — будуємо її при першому виклику
static void buildCRC32()
{
    if (g_crcReady) return;
    for (DWORD i = 0; i < 256; i++)
    {
        DWORD c = i;
        for (int j = 0; j < 8; j++)
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        g_crcTable[i] = c;
    }
    g_crcReady = true;
}

// Обчислення CRC32 для даних довільного розміру
static DWORD calcCRC32(const BYTE* data, DWORD len)
{
    buildCRC32();
    DWORD crc = 0xFFFFFFFFu;
    for (DWORD i = 0; i < len; i++)
        crc = g_crcTable[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

// CRC рахується тільки по перших 12 байтах (msgCount + totalSize + maxSize)
static DWORD calcHeaderCRC(const MailboxHeader& hdr)
{
    return calcCRC32((const BYTE*)&hdr, 12);
}

// ======================================================
// Допоміжні функції
// ======================================================

static void printSep(const char* title)
{
    printf("\n============================================================\n");
    printf("%s\n", title);
    printf("============================================================\n");
}

static string msgPath(DWORD idx)
{
    ostringstream oss;
    oss << "Mailbox\\messages\\msg_" << setw(4) << setfill('0') << idx << ".bin";
    return oss.str();
}

// Встановлюємо розмір файлу перед відображенням —
// CreateFileMapping вимагає ненульового розміру файлу
static bool setFileSize(HANDLE hFile, DWORD sz)
{
    SetFilePointer(hFile, sz, NULL, FILE_BEGIN);
    return SetEndOfFile(hFile) != 0;
}

// ======================================================
// Читання / запис заголовку через відображення файлу
// ======================================================

static bool readHeader(MailboxHeader& hdr)
{
    HANDLE hFile = CreateFileA(MAILBOX_META,
        GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    // Розмір 0 у CreateFileMapping = відобразити увесь файл
    HANDLE hMap = CreateFileMappingA(hFile, NULL,
        PAGE_READONLY, 0, sizeof(MailboxHeader), NULL);
    if (!hMap) { CloseHandle(hFile); return false; }

    const void* pView = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(MailboxHeader));
    if (!pView) { CloseHandle(hMap); CloseHandle(hFile); return false; }

    memcpy(&hdr, pView, sizeof(MailboxHeader));

    UnmapViewOfFile(pView);
    CloseHandle(hMap);
    CloseHandle(hFile);
    return true;
}

static bool writeHeader(const MailboxHeader& hdr)
{
    HANDLE hFile = CreateFileA(MAILBOX_META,
        GENERIC_READ | GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    if (!setFileSize(hFile, sizeof(MailboxHeader)))
    {
        CloseHandle(hFile); return false;
    }
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

    HANDLE hMap = CreateFileMappingA(hFile, NULL,
        PAGE_READWRITE, 0, sizeof(MailboxHeader), NULL);
    if (!hMap) { CloseHandle(hFile); return false; }

    void* pView = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, sizeof(MailboxHeader));
    if (!pView) { CloseHandle(hMap); CloseHandle(hFile); return false; }

    memcpy(pView, &hdr, sizeof(MailboxHeader));

    // FlushViewOfFile скидає зміни з кешу відображення на диск
    FlushViewOfFile(pView, sizeof(MailboxHeader));
    UnmapViewOfFile(pView);
    CloseHandle(hMap);
    CloseHandle(hFile);
    return true;
}

// ======================================================
// Операції зі скринькою
// ======================================================

static bool createMailbox(DWORD maxSizeBytes)
{
    if (!CreateDirectoryA("Mailbox", NULL) &&
        GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Помилка: не вдалось створити каталог Mailbox\n");
        return false;
    }
    if (!CreateDirectoryA("Mailbox\\messages", NULL) &&
        GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Помилка: не вдалось створити каталог messages\n");
        return false;
    }

    MailboxHeader hdr = {};
    hdr.maxSize = maxSizeBytes;
    hdr.crc32 = calcHeaderCRC(hdr);

    if (!writeHeader(hdr))
    {
        printf("Помилка: не вдалось записати мета-файл\n");
        return false;
    }

    printf("Поштову скриньку створено. Максимальний розмiр: %u байт\n", maxSizeBytes);
    return true;
}

static DWORD findFreeIndex()
{
    DWORD idx = 0;
    while (GetFileAttributesA(msgPath(idx).c_str()) != INVALID_FILE_ATTRIBUTES)
        idx++;
    return idx;
}

// Записуємо лист через MapViewOfFile замість WriteFile
static bool addMessage(const string& body)
{
    MailboxHeader hdr;
    if (!readHeader(hdr)) { printf("Помилка читання заголовку\n"); return false; }

    DWORD msgSize = (DWORD)body.size();
    if (hdr.totalSize + msgSize > hdr.maxSize)
    {
        printf("Помилка: скринька переповнена (%u + %u > %u)\n",
            hdr.totalSize, msgSize, hdr.maxSize);
        return false;
    }

    DWORD  idx = findFreeIndex();
    string path = msgPath(idx);
    DWORD  fileSize = sizeof(DWORD) + msgSize;  // 4 байти розміру + тіло

    HANDLE hFile = CreateFileA(path.c_str(),
        GENERIC_READ | GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Помилка: не вдалось створити файл повiдомлення\n");
        return false;
    }

    if (!setFileSize(hFile, fileSize)) { CloseHandle(hFile); return false; }
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

    HANDLE hMap = CreateFileMappingA(hFile, NULL,
        PAGE_READWRITE, 0, fileSize, NULL);
    if (!hMap) { CloseHandle(hFile); return false; }

    BYTE* pView = (BYTE*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, fileSize);
    if (!pView) { CloseHandle(hMap); CloseHandle(hFile); return false; }

    // Пишемо через вказівник — без ReadFile/WriteFile
    memcpy(pView, &msgSize, sizeof(DWORD));
    memcpy(pView + sizeof(DWORD), body.c_str(), msgSize);

    FlushViewOfFile(pView, fileSize);
    UnmapViewOfFile(pView);
    CloseHandle(hMap);
    CloseHandle(hFile);

    hdr.msgCount++;
    hdr.totalSize += msgSize;
    hdr.crc32 = calcHeaderCRC(hdr);
    if (!writeHeader(hdr)) { printf("Помилка оновлення заголовку\n"); return false; }

    printf("Лист #%u додано (%u байт)\n", idx, msgSize);
    return true;
}

// Читаємо тіло листа через MapViewOfFile
static bool readMessage(DWORD idx, bool deleteAfterRead)
{
    string path = msgPath(idx);

    HANDLE hFile = CreateFileA(path.c_str(),
        GENERIC_READ, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Помилка: лист #%u не знайдено\n", idx);
        return false;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);

    HANDLE hMap = CreateFileMappingA(hFile, NULL,
        PAGE_READONLY, 0, fileSize, NULL);
    if (!hMap) { CloseHandle(hFile); return false; }

    const BYTE* pView = (const BYTE*)MapViewOfFile(
        hMap, FILE_MAP_READ, 0, 0, fileSize);
    if (!pView) { CloseHandle(hMap); CloseHandle(hFile); return false; }

    DWORD msgSize = 0;
    memcpy(&msgSize, pView, sizeof(DWORD));

    string body(msgSize, '\0');
    memcpy(&body[0], pView + sizeof(DWORD), msgSize);

    UnmapViewOfFile(pView);
    CloseHandle(hMap);
    CloseHandle(hFile);

    printf("--- Лист #%u (%u байт) ---\n%s\n", idx, msgSize, body.c_str());

    if (deleteAfterRead)
    {
        DeleteFileA(path.c_str());

        MailboxHeader hdr;
        if (readHeader(hdr))
        {
            if (hdr.msgCount > 0)        hdr.msgCount--;
            if (hdr.totalSize >= msgSize)  hdr.totalSize -= msgSize;
            hdr.crc32 = calcHeaderCRC(hdr);
            writeHeader(hdr);
        }
        printf("(Лист #%u видалено пiсля читання)\n", idx);
    }

    return true;
}

// Зчитуємо розмір через відображення, щоб оновити лічильник після видалення
static bool deleteMessage(DWORD idx)
{
    string path = msgPath(idx);

    HANDLE hFile = CreateFileA(path.c_str(),
        GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Помилка: лист #%u не знайдено\n", idx);
        return false;
    }

    HANDLE hMap = CreateFileMappingA(hFile, NULL,
        PAGE_READONLY, 0, sizeof(DWORD), NULL);
    if (!hMap) { CloseHandle(hFile); return false; }

    const DWORD* pView = (const DWORD*)MapViewOfFile(
        hMap, FILE_MAP_READ, 0, 0, sizeof(DWORD));
    DWORD msgSize = pView ? *pView : 0;
    if (pView) UnmapViewOfFile(pView);
    CloseHandle(hMap);
    CloseHandle(hFile);

    if (!DeleteFileA(path.c_str()))
    {
        printf("Помилка видалення листа #%u\n", idx);
        return false;
    }

    MailboxHeader hdr;
    if (readHeader(hdr))
    {
        if (hdr.msgCount > 0)        hdr.msgCount--;
        if (hdr.totalSize >= msgSize)  hdr.totalSize -= msgSize;
        hdr.crc32 = calcHeaderCRC(hdr);
        writeHeader(hdr);
    }

    printf("Лист #%u видалено\n", idx);
    return true;
}

static bool deleteAllMessages()
{
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("Mailbox\\messages\\msg_*.bin", &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("Листiв немає — скринька вже порожня\n");
        return true;
    }

    DWORD deleted = 0;
    do
    {
        string full = string("Mailbox\\messages\\") + fd.cFileName;
        if (DeleteFileA(full.c_str())) deleted++;
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);

    MailboxHeader hdr;
    if (readHeader(hdr))
    {
        hdr.msgCount = 0;
        hdr.totalSize = 0;
        hdr.crc32 = calcHeaderCRC(hdr);
        writeHeader(hdr);
    }

    printf("Видалено %u листiв\n", deleted);
    return true;
}

// Перевірка через CRC32 і реальний підрахунок файлів
static bool checkIntegrity()
{
    MailboxHeader hdr;
    if (!readHeader(hdr))
    {
        printf("Помилка читання заголовку при перевiрцi цiлiсностi\n");
        return false;
    }

    DWORD expected = calcHeaderCRC(hdr);
    if (expected != hdr.crc32)
    {
        printf("ЦIЛIСНIСТЬ ПОРУШЕНА! CRC збережений=0x%08X, розрахований=0x%08X\n",
            hdr.crc32, expected);
        return false;
    }

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("Mailbox\\messages\\msg_*.bin", &fd);
    DWORD realCount = 0, realSize = 0;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            string full = string("Mailbox\\messages\\") + fd.cFileName;
            HANDLE hF = CreateFileA(full.c_str(),
                GENERIC_READ, FILE_SHARE_READ, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hF != INVALID_HANDLE_VALUE)
            {
                HANDLE hM = CreateFileMappingA(hF, NULL,
                    PAGE_READONLY, 0, sizeof(DWORD), NULL);
                if (hM)
                {
                    const DWORD* pV = (const DWORD*)MapViewOfFile(
                        hM, FILE_MAP_READ, 0, 0, sizeof(DWORD));
                    if (pV) { realSize += *pV; UnmapViewOfFile(pV); }
                    CloseHandle(hM);
                }
                CloseHandle(hF);
                realCount++;
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }

    bool ok = (realCount == hdr.msgCount) && (realSize == hdr.totalSize);
    if (ok)
        printf("Цiлiснiсть OK: CRC=0x%08X, листiв=%u, розмiр=%u байт\n",
            hdr.crc32, hdr.msgCount, hdr.totalSize);
    else
        printf("НЕВIДПОВIДНIСТЬ! Заголовок: %u/%u; Реально: %u/%u\n",
            hdr.msgCount, hdr.totalSize, realCount, realSize);

    return ok;
}

static void printMailboxInfo()
{
    MailboxHeader hdr;
    if (!readHeader(hdr)) { printf("Помилка читання заголовку\n"); return; }

    printf("  Кiлькiсть листiв   : %u\n", hdr.msgCount);
    printf("  Загальний розмiр   : %u байт\n", hdr.totalSize);
    printf("  Максимальний розмiр: %u байт\n", hdr.maxSize);
    printf("  CRC32 заголовку    : 0x%08X\n", hdr.crc32);
}

// ======================================================
// main
// ======================================================

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printSep("| Завдання 10 - Поштова скринька через вiдображення файлiв |");

    printf("1. Створення скриньки (макс. 4096 байт)\n");
    if (!createMailbox(4096)) return 1;

    printf("\n2. Додавання листiв\n");
    addMessage("Лист 1: вiдображення файлiв через CreateFileMapping + MapViewOfFile.");
    addMessage("Лист 2: MapViewOfFile повертає вказiвник — пам'ять i файл синхронiзованi.");
    addMessage("Лист 3: FlushViewOfFile забезпечує запис змiн на диск.");
    addMessage("Лист 4: Короткий тест.");

	// Щоб зробити скрін що файли створено
	getchar();

    printf("\n3. Стан скриньки пiсля додавання\n");
    printMailboxInfo();

    printf("\n4. Читання листа #0 (без видалення)\n");
    readMessage(0, false);

    printf("\n5. Читання листа #1 (з видаленням)\n");
    readMessage(1, true);

    printf("\n6. Стан пiсля читання з видаленням\n");
    printMailboxInfo();

    printf("\n7. Видалення листа #2\n");
    deleteMessage(2);
    printMailboxInfo();

    printf("\n8. Перевiрка цiлiсностi (CRC32)\n");
    checkIntegrity();

    printf("\n9. Видалення всiх листiв\n");
    deleteAllMessages();

    printf("\n10. Фiнальна перевiрка цiлiсностi\n");
    checkIntegrity();
    return 0;
}