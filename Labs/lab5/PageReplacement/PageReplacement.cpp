#include <windows.h>
#include <stdio.h>
#include <vector>
#include <climits>

using namespace std;

static void printSep(const char* title)
{
    printf("\n=========================================================\n");
    printf("%s\n", title);
    printf("=========================================================\n");
}

// ======================================================
// Алгоритм FIFO: витісняємо ту сторінку, що прийшла найраніше
// ======================================================

static int fifo(const vector<int>& pages, int frames, bool verbose)
{
    vector<int> frame(frames, -1);
    int oldest = 0;
    int faults = 0;

    for (int i = 0; i < (int)pages.size(); i++)
    {
        int  pg = pages[i];
        bool hit = false;
        for (int f : frame)
            if (f == pg) { hit = true; break; }

        if (!hit)
        {
            frame[oldest] = pg;
            oldest = (oldest + 1) % frames;
            faults++;
        }

        if (verbose)
        {
            printf("  Крок %2d  стор.%2d  |", i + 1, pg);
            for (int j = 0; j < frames; j++)
                (frame[j] == -1) ? printf("  - ") : printf(" %2d ", frame[j]);
            printf("| %s\n", hit ? "       |" : "ПРОМАХ |");
        }
    }
    return faults;
}

// ======================================================
// Алгоритм LRU: витісняємо сторінку, яка найдовше не використовувалась
// ======================================================

static int lru(const vector<int>& pages, int frames, bool verbose)
{
    vector<int> frame(frames, -1);
    vector<int> lastUse(frames, 0);
    int faults = 0;

    for (int t = 0; t < (int)pages.size(); t++)
    {
        int  pg = pages[t];
        bool hit = false;
        int  hitIdx = -1;

        for (int i = 0; i < frames; i++)
            if (frame[i] == pg) { hit = true; hitIdx = i; break; }

        if (!hit)
        {
            // Шукаємо порожній слот або той, що використовувався найдавніше
            int replaceIdx = 0;
            for (int i = 0; i < frames; i++)
            {
                if (frame[i] == -1) { replaceIdx = i; break; }
                if (lastUse[i] < lastUse[replaceIdx])      replaceIdx = i;
            }
            frame[replaceIdx] = pg;
            lastUse[replaceIdx] = t;
            faults++;
        }
        else
        {
            lastUse[hitIdx] = t;
        }

        if (verbose)
        {
            printf("  Крок %2d  стор.%2d  |", t + 1, pg);
            for (int j = 0; j < frames; j++)
                (frame[j] == -1) ? printf("  - ") : printf(" %2d ", frame[j]);
            printf("| %s\n", hit ? "       |" : "ПРОМАХ |");
        }
    }
    return faults;
}

// ======================================================
// Алгоритм OPT: витісняємо сторінку, яка не знадобиться найдовше
// ======================================================

static int opt(const vector<int>& pages, int frames, bool verbose)
{
    vector<int> frame(frames, -1);
    int faults = 0;

    for (int i = 0; i < (int)pages.size(); i++)
    {
        int  pg = pages[i];
        bool hit = false;

        for (int f : frame)
            if (f == pg) { hit = true; break; }

        if (!hit)
        {
            int replaceIdx = 0;
            int farthest = -1;

            for (int j = 0; j < frames; j++)
            {
                if (frame[j] == -1) { replaceIdx = j; break; }

                // Знаходимо наступне звернення до frame[j] в майбутньому
                int nextUse = INT_MAX;
                for (int k = i + 1; k < (int)pages.size(); k++)
                    if (pages[k] == frame[j]) { nextUse = k; break; }

                // Обираємо слот з найдальшим наступним зверненням
                if (nextUse > farthest) { farthest = nextUse; replaceIdx = j; }
            }

            frame[replaceIdx] = pg;
            faults++;
        }

        if (verbose)
        {
            printf("  Крок %2d  стор.%2d  |", i + 1, pg);
            for (int j = 0; j < frames; j++)
                (frame[j] == -1) ? printf("  - ") : printf(" %2d ", frame[j]);
            printf("| %s\n", hit ? "       |" : "ПРОМАХ |");
        }
    }
    return faults;
}

// ======================================================
// main
// ======================================================

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printSep("| Завдання 1 - Алгоритми замiщення сторiнок           |");

    // Класичний рядок звернень із теорії ОС, 3 фрейми
    const vector<int> ref = { 7, 0, 1, 2, 0, 3, 0, 4, 2, 3,
                               0, 3, 2, 1, 2, 0, 1, 7, 0, 1 };
    const int FRAMES = 3;

    printf("  Рядок звернень: ");
    for (int p : ref) printf("%d ", p);
    printf("\n  Кiлькiсть фреймiв: %d\n", FRAMES);

    printf("\n--- FIFO -----------------------------------\n");
    int f = fifo(ref, FRAMES, true);
	printf("--------------------------------------------\n");
    printf("  Промахiв FIFO: %d\n", f);

    printf("\n--- LRU ------------------------------------\n");
    int l = lru(ref, FRAMES, true);
	printf("--------------------------------------------\n");
    printf("  Промахiв LRU: %d\n", l);

    printf("\n--- OPT ------------------------------------\n");
    int o = opt(ref, FRAMES, true);
	printf("--------------------------------------------\n");
    printf("  Промахiв OPT: %d\n", o);

    // OPT є теоретичним мiнiмумом; LRU зазвичай наближається до нього
    printf("\n[Порiвняння] FIFO=%d   LRU=%d   OPT=%d\n", f, l, o);

    return 0;
}