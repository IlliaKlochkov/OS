#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <string.h>

// потрібно для GetModuleInformation, який дає точний розмір образу через SizeOfImage
#pragma comment(lib, "psapi.lib")

// ── UTF-8 aware padding ───────────────────────────────────────────────────────
// printf("%-Ns", str) рахує байти, а не символи.
// Кирилиця = 2 байти, але 1 екранний символ — тому таблиця їде.
// Ця функція рахує саме екранну ширину і доповнює пробілами правильно.

static int screenWidth(const char* s)
{
    int w = 0;
    const unsigned char* p = (const unsigned char*)s;
    while (*p)
    {
        if ((*p & 0x80) == 0x00) p += 1;   // ASCII (1 байт)
        else if ((*p & 0xE0) == 0xC0) p += 2;   // кирилиця та інші (2 байти)
        else if ((*p & 0xF0) == 0xE0) p += 3;   // 3-байтні символи
        else                           p += 4;   // 4-байтні символи
        w++;
    }
    return w;
}

static void padPrint(const char* s, int width)
{
    printf("%s", s);
    int pad = width - screenWidth(s);
    for (int i = 0; i < pad; i++) printf(" ");
}

// ── Розміри стовпців (у екранних символах) ───────────────────────────────────
#define W_TAG    6
#define W_BASE   18
#define W_SIZE   13
#define W_STATE  11
#define W_PROT   14
#define W_NOTE   38

static void tableRule()
{
    printf("  +");
    for (int i = 0; i < W_TAG + 2; i++) printf("-"); printf("+");
    for (int i = 0; i < W_BASE + 2; i++) printf("-"); printf("+");
    for (int i = 0; i < W_SIZE + 2; i++) printf("-"); printf("+");
    for (int i = 0; i < W_STATE + 2; i++) printf("-"); printf("+");
    for (int i = 0; i < W_PROT + 2; i++) printf("-"); printf("+");
    for (int i = 0; i < W_NOTE + 2; i++) printf("-"); printf("+\n");
}

static void tableHeader(const char* c1, const char* c2, const char* c3,
    const char* c4, const char* c5, const char* c6)
{
    tableRule();
    printf("  | "); padPrint(c1, W_TAG);
    printf(" | "); padPrint(c2, W_BASE);
    printf(" | "); padPrint(c3, W_SIZE);
    printf(" | "); padPrint(c4, W_STATE);
    printf(" | "); padPrint(c5, W_PROT);
    printf(" | "); padPrint(c6, W_NOTE);
    printf(" |\n");
    tableRule();
}

static void tableRow(const char* tag, LPCVOID addr, const char* note)
{
    MEMORY_BASIC_INFORMATION mbi = {};
    if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0) return;

    const char* stateStr =
        (mbi.State == MEM_FREE) ? "MEM_FREE" :
        (mbi.State == MEM_COMMIT) ? "MEM_COMMIT" :
        (mbi.State == MEM_RESERVE) ? "MEM_RESERVE" : "???";

    const char* protStr =
        (mbi.Protect == PAGE_READWRITE) ? "PAGE_READWRITE" :
        (mbi.Protect == PAGE_READONLY) ? "PAGE_READONLY" :
        (mbi.Protect == PAGE_NOACCESS) ? "PAGE_NOACCESS" :
        (mbi.Protect == PAGE_EXECUTE_READ) ? "PAGE_EXEC_READ" :
        (mbi.Protect == PAGE_EXECUTE_READWRITE) ? "PAGE_EXEC_RW" :
        (mbi.Protect == 0) ? "---" : "???";

    char sizeBuf[24], addrBuf[24];
    double mb = (double)mbi.RegionSize / (1024.0 * 1024.0);
    if (mb >= 1024.0)
        snprintf(sizeBuf, sizeof(sizeBuf), "%.2f GB", mb / 1024.0);
    else
        snprintf(sizeBuf, sizeof(sizeBuf), "%.2f MB", mb);
    snprintf(addrBuf, sizeof(addrBuf), "%p", mbi.BaseAddress);

    printf("  | "); padPrint(tag, W_TAG);
    printf(" | "); padPrint(addrBuf, W_BASE);
    printf(" | "); padPrint(sizeBuf, W_SIZE);
    printf(" | "); padPrint(stateStr, W_STATE);
    printf(" | "); padPrint(protStr, W_PROT);
    printf(" | "); padPrint(note, W_NOTE);
    printf(" |\n");
}

static void printSep(const char* title)
{
    printf("\n  ============================================================\n");
    printf("  %s\n", title);
    printf("  ============================================================\n");
}

// ======================================================
// Завдання 7 — VirtualQuery: список вільних блоків
// ======================================================

static void task7_virtualQuery()
{
    printSep("| Завдання 7 - VirtualQuery: список вiльних блокiв         |");

    SYSTEM_INFO si = {};
    GetSystemInfo(&si);

    printf("\n  Параметри системи:\n");
    printf("  Розмiр сторiнки         : %u байт\n", si.dwPageSize);
    printf("  Гранулярнiсть виділення : %u КБ\n", si.dwAllocationGranularity / 1024);
    printf("  Мiн. адреса застосунку  : %p\n", si.lpMinimumApplicationAddress);
    printf("  Макс. адреса застосунку : %p\n", si.lpMaximumApplicationAddress);

    BYTE* addr = (BYTE*)si.lpMinimumApplicationAddress;
    BYTE* maxAddr = (BYTE*)si.lpMaximumApplicationAddress;
    int    total = 0;
    SIZE_T freeSum = 0;
    int    shown = 0;

    const SIZE_T SHOW_MIN = 1ULL * 1024 * 1024 * 1024;  // показуємо >= 1 ГБ

    printf("\n  Вiльнi регiони >= 1 ГБ:\n");
    tableHeader("Тег", "Базова адреса", "Розмiр", "Стан", "Захист", "Примiтка");

    MEMORY_BASIC_INFORMATION mbi = {};
    while (addr < maxAddr)
    {
        if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0) break;

        if (mbi.State == MEM_FREE)
        {
            total++;
            freeSum += mbi.RegionSize;

            if (mbi.RegionSize >= SHOW_MIN && shown < 12)
            {
                char tag[8];
                snprintf(tag, sizeof(tag), "#%d", shown + 1);
                tableRow(tag, addr, "");
                shown++;
            }
        }

        addr += mbi.RegionSize;
    }

    tableRule();
    printf("  Всього вiльних регiонiв : %d\n", total);
    printf("  Загальний вiльний обсяг : %.2f ГБ\n",
        (double)freeSum / (1024.0 * 1024.0 * 1024.0));
}

// ======================================================
// Завдання 8 — VirtualAlloc: наочна демонстрація змін
// ======================================================

static void task8_virtualAlloc()
{
    printSep("| Завдання 8 - VirtualAlloc: змiни у списку вiльних блокiв |");

    const SIZE_T ALLOC_SIZE = 64ULL * 1024 * 1024;

    SYSTEM_INFO si = {};
    GetSystemInfo(&si);

    // Знаходимо перший вільний блок >= 64 МБ і передаємо його адресу у VirtualAlloc —
    // так ДО і ПІСЛЯ гарантовано показують той самий блок
    BYTE* addr = (BYTE*)si.lpMinimumApplicationAddress;
    BYTE* maxAddr = (BYTE*)si.lpMaximumApplicationAddress;

    MEMORY_BASIC_INFORMATION mbi = {};
    LPVOID targetBase = NULL;
    SIZE_T targetSize = 0;

    while (addr < maxAddr)
    {
        if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0) break;
        if (mbi.State == MEM_FREE && mbi.RegionSize >= ALLOC_SIZE)
        {
            targetBase = mbi.BaseAddress;
            targetSize = mbi.RegionSize;
            break;
        }
        addr += mbi.RegionSize;
    }

    if (!targetBase)
    {
        printf("  Не знайдено вiльного блоку >= 64 МБ\n");
        return;
    }

    // ── ДО ───────────────────────────────────────────────────────────────────
    printf("\n  Стан ДО виділення:\n");
    tableHeader("Тег", "Базова адреса", "Розмiр", "Стан", "Захист", "Примiтка");
    tableRow("ДО", targetBase, "перший придатний вiльний блок");
    tableRule();

    // ── Виділення за конкретною адресою ──────────────────────────────────────
    void* pMem = VirtualAlloc(targetBase, ALLOC_SIZE,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pMem)
    {
        printf("  (ОС вiдмовила за цiєю адресою, пробуємо NULL...)\n");
        pMem = VirtualAlloc(NULL, ALLOC_SIZE,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!pMem)
        {
            printf("  Помилка VirtualAlloc: %lu\n", GetLastError());
            return;
        }
        targetBase = pMem;
        targetSize = ALLOC_SIZE;
    }

    // ── ПІСЛЯ: блок розбився на два ──────────────────────────────────────────
    BYTE* afterAlloc = (BYTE*)pMem + ALLOC_SIZE;

    printf("\n  Стан ПIСЛЯ виділення — вихiдний блок розбито надвоє:\n");
    tableHeader("Тег", "Базова адреса", "Розмiр", "Стан", "Захист", "Примiтка");
    tableRow("ALLOC", pMem, "виділено нами");
    tableRow("ЗАЛИШ", afterAlloc, "залишок вихiдного блоку");
    tableRule();

    printf("  %.2f МБ  ->  64.00 МБ (COMMIT)  +  %.2f МБ (FREE)\n",
        (double)targetSize / (1024.0 * 1024.0),
        (double)(targetSize - ALLOC_SIZE) / (1024.0 * 1024.0));

    printf("\n  Стратегiя ОС: First-Fit\n");
    printf("  VirtualAlloc знайшов перший блок >= 64 МБ i розбив його.\n");
    printf("  Гранулярнiсть: %u КБ — виділення завжди кратне їй.\n",
        si.dwAllocationGranularity / 1024);

    // ── Звільнення ────────────────────────────────────────────────────────────
    VirtualFree(pMem, 0, MEM_RELEASE);

    printf("\n  Стан ПIСЛЯ VirtualFree:\n");
    tableHeader("Тег", "Базова адреса", "Розмiр", "Стан", "Захист", "Примiтка");
    tableRow("FREE", pMem, "блок повернуто ОС");
    tableRule();
}

// ======================================================
// main
// ======================================================

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    task7_virtualQuery();
    task8_virtualAlloc();

    return 0;
}