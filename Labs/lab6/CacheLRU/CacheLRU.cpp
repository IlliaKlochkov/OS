#include <windows.h>
#include <stdio.h>
#include <vector>

using namespace std;

// стандартні параметри кешу:
#define BLOCK_SIZE  64     // b: розмір блоку в байтах -> offset = 6 біт
#define LINES      128     // l: кількість рядків кешу -> line = 7 біт
#define WAYS         4     // n: блоків у рядку (4-асоціативний кеш)
// загальний розмір = 64 * 4 * 128 = 32 КБ

static void printSep(const char* title)
{
    printf("\n=========================================================\n");
    printf("%s\n", title);
    printf("=========================================================\n");
}

// блок у рядку кешу (way)
struct Way
{
    bool valid;
    UINT tag;
    int  lastUse;   // момент останнього звернення (для LRU)
};


// прогін трасою адрес з витісненням за алгоритмом LRU. Розклад адреси A: offset = A & 0x3F, line = (A >> 6) & 0x7F, tag = A >> 13. Повертає кількість промахів.

static int lruCache(const vector<UINT>& trace, bool verbose)
{
    Way cache[LINES][WAYS];
    ZeroMemory(cache, sizeof(cache));

    int hits = 0;
    int misses = 0;

    for (int t = 0; t < (int)trace.size(); t++)
    {
        UINT A = trace[t];
        UINT line = (A >> 6) & 0x7F;
        UINT tag = A >> 13;

        Way* set = cache[line];

        // пошук влучення в рядку
        bool hit = false;
        for (int w = 0; w < WAYS; w++)
            if (set[w].valid && set[w].tag == tag) { hit = true; set[w].lastUse = t; break; }

        if (!hit)
        {
            // вибір way для витіснення: вільний або найменш використовуваний
            int repl = 0;
            for (int w = 0; w < WAYS; w++)
            {
                if (!set[w].valid) { 
                    repl = w; 
                    break; 
                }
                if (set[w].lastUse < set[repl].lastUse) repl = w;
            }

            set[repl].valid = true;
            set[repl].tag = tag;
            set[repl].lastUse = t;
            misses++;
        }
        else
            hits++;

        if (verbose)
        {
            // вивід стану рядка після звернення
            printf("  крок %2d  A=0x%05X  line=%3u  |", t + 1, A, line);
            for (int w = 0; w < WAYS; w++)
                set[w].valid ? printf(" %04X", set[w].tag) : printf("  -  ");
            printf("| %s\n", hit ? "влучення" : "ПРОМАХ  ");
        }
    }

    int total = hits + misses;
    printf("  Влучень: %d, промахів: %d", hits, misses);
    if (total > 0) printf(", hit-rate: %.1f%%", 100.0 * hits / total);
    printf("\n");

    return misses;
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printSep("| Завдання 3 - Алгоритм LRU для кеш-пам'яті           |");
    printf("  Параметри: b=%d байт, l=%d рядків, n=%d way -> кеш %d КБ\n",
        BLOCK_SIZE, LINES, WAYS, BLOCK_SIZE * LINES * WAYS / 1024);

    // сценарій 1: переповнення рядка (адреси кратні 0x2000 мають однаковий line, але різний tag -> LRU)
    printf("\n--- Сценарій 1: переповнення рядка ----------\n");
    vector<UINT> s1 = { 0x00000, 0x02000, 0x04000, 0x06000, 0x00010, 0x08000, 0x02000 };
    lruCache(s1, true);

    // сценарій 2: часова локальність (повторні звернення)
    printf("\n--- Сценарій 2: часова локальність ----------\n");
    vector<UINT> s2 = { 0x01000, 0x01004, 0x01008, 0x01000, 0x01004, 0x01008, 0x0100C };
    lruCache(s2, true);

    // сценарій 3: просторова локальність (послідовний обхід в межах блоку)
    printf("\n--- Сценарій 3: просторова локальність ------\n");
    vector<UINT> s3;
    for (UINT off = 0; off < BLOCK_SIZE; off += 16) s3.push_back(0x03000 + off);
    lruCache(s3, true);

    return 0;
}
