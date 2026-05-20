#include <windows.h>
#include <stdio.h>

static void printSep(const char* title)
{
    printf("\n====================================================================\n");
    printf("%s\n", title);
    printf("====================================================================\n");
}

// ======================================================
// Заголовок блоку кастомного хіпу
// Розміщується безпосередньо перед корисними даними
// ======================================================

struct HeapBlock
{
    SIZE_T dataSize;  // розмір корисних даних (без заголовка)
    BOOL   isFree;    // TRUE — вільний, FALSE — зайнятий
};

static BYTE* g_heapBase = NULL;
static SIZE_T g_heapSize = 0;

// ======================================================
// Ініціалізація: виділяємо регіон VirtualAlloc і
// оголошуємо його одним великим вільним блоком
// ======================================================

static bool heapInit(SIZE_T size)
{
    g_heapBase = (BYTE*)VirtualAlloc(NULL, size,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!g_heapBase) return false;

    g_heapSize = size;

    HeapBlock* first = (HeapBlock*)g_heapBase;
    first->dataSize = size - sizeof(HeapBlock);
    first->isFree = TRUE;

    return true;
}

// ======================================================
// Best-Fit alloc: шукаємо найменший вільний блок,
// що вміщує запит — мінімізує залишки і фрагментацію
// ======================================================

static void* heapAlloc(SIZE_T size)
{
    // Вирівнюємо до 8 байт, щоб уникнути проблем з вирівнюванням структур
    SIZE_T aligned = (size + 7) & ~(SIZE_T)7;

    HeapBlock* best = NULL;
    SIZE_T     bestSize = (SIZE_T)-1;

    BYTE* cur = g_heapBase;
    while (cur < g_heapBase + g_heapSize)
    {
        HeapBlock* blk = (HeapBlock*)cur;

        if (blk->isFree && blk->dataSize >= aligned && blk->dataSize < bestSize)
        {
            bestSize = blk->dataSize;
            best = blk;
        }

        cur += sizeof(HeapBlock) + blk->dataSize;
    }

    if (!best) return NULL;

    // Якщо залишок достатній для нового заголовка + мінімального блоку — ділимо
    SIZE_T remainder = best->dataSize - aligned;
    if (remainder > sizeof(HeapBlock) + 8)
    {
        HeapBlock* next = (HeapBlock*)((BYTE*)best + sizeof(HeapBlock) + aligned);
        next->dataSize = remainder - sizeof(HeapBlock);
        next->isFree = TRUE;
        best->dataSize = aligned;
    }

    best->isFree = FALSE;
    return (BYTE*)best + sizeof(HeapBlock);
}

// ======================================================
// Free + coalescing: зливаємо суміжні вільні блоки,
// щоб не накопичувалась зовнішня фрагментація
// ======================================================

static void heapFree(void* ptr)
{
    if (!ptr) return;

    HeapBlock* blk = (HeapBlock*)((BYTE*)ptr - sizeof(HeapBlock));
    blk->isFree = TRUE;

    // Повторюємо злиття до тих пір, поки є що зливати
    bool merged;
    do
    {
        merged = false;
        BYTE* cur = g_heapBase;

        while (cur < g_heapBase + g_heapSize)
        {
            HeapBlock* a = (HeapBlock*)cur;
            BYTE* next = cur + sizeof(HeapBlock) + a->dataSize;

            if (next < g_heapBase + g_heapSize)
            {
                HeapBlock* b = (HeapBlock*)next;
                if (a->isFree && b->isFree)
                {
                    // Поглинаємо блок b всередину блоку a
                    a->dataSize += sizeof(HeapBlock) + b->dataSize;
                    merged = true;
                    break;
                }
            }

            cur += sizeof(HeapBlock) + a->dataSize;
        }
    } while (merged);
}

// ======================================================
// Виводимо карту хіпу для наочності
// ======================================================

static void heapPrint(const char* label)
{
    printf("[%s]\n", label);
    BYTE* cur = g_heapBase;
    int   idx = 0;

    while (cur < g_heapBase + g_heapSize)
    {
        HeapBlock* blk = (HeapBlock*)cur;
        printf("  Блок %d: адр=%p  розмiр=%5llu б  %s\n",
            idx++,
            (void*)(cur + sizeof(HeapBlock)),
            (unsigned long long)blk->dataSize,
            blk->isFree ? "ВIЛЬНИЙ" : "ЗАЙНЯТИЙ");
        cur += sizeof(HeapBlock) + blk->dataSize;
    }
}

static void heapDestroy()
{
    if (g_heapBase)
    {
        VirtualFree(g_heapBase, 0, MEM_RELEASE);
        g_heapBase = NULL;
        g_heapSize = 0;
    }
}

// ======================================================
// main
// ======================================================

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printSep("| Завдання 9 - Алокатор: стратегiя \"Найменший достатнiй\" (Best-Fit)|");

    const SIZE_T HEAP_SZ = 4096;

    if (!heapInit(HEAP_SZ))
    {
        printf("Помилка iнiцiалiзацiї хiпу\n");
        return 1;
    }
    printf("Хiп iнiцiалiзовано: %llu байт\n\n", (unsigned long long)HEAP_SZ);

    heapPrint("початковий стан");

    // Виділяємо три блоки різного розміру — навмисно створюємо фрагментацію
    void* p1 = heapAlloc(100);
    printf("\nheapAlloc(100) => %p\n", p1);
    heapPrint("пiсля alloc(100)");

    void* p2 = heapAlloc(200);
    printf("\nheapAlloc(200) => %p\n", p2);
    heapPrint("пiсля alloc(200)");

    void* p3 = heapAlloc(50);
    printf("\nheapAlloc(50)  => %p\n", p3);
    heapPrint("пiсля alloc(50)");

    // Звільняємо середній блок — утворюється «дірка» на 200 б
    heapFree(p2);
    printf("\nheapFree(p2) [200 б]\n");
    heapPrint("пiсля free(p2)");

    // Запит 80 б: дірка 200 б i великий хвiст обидва пiдходять,
    // Best-Fit вибере меншу — 200 б, щоб зберегти великий блок для бiльших запитiв
    void* p4 = heapAlloc(80);
    printf("\nheapAlloc(80) — Best-Fit обирає дiрку 200 б => %p\n", p4);
    heapPrint("пiсля alloc(80)");

    // Звільняємо все — перевіряємо злиття в один блок
    heapFree(p1);
    heapFree(p4);
    heapFree(p3);
    printf("\nheapFree(p1, p4, p3) — злиття сумiжних вiльних блокiв\n");
    heapPrint("фiнальний стан");

    heapDestroy();
    return 0;
}