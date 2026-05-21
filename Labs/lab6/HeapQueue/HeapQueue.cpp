// Лабораторна робота №6 — Завдання 2
// Клас черги (FIFO), побудований на функціях роботи з купою:
// HeapCreate (приватна купа) / HeapAlloc / HeapFree / HeapSize / HeapDestroy.
// Кожен вузол черги виділяється окремо у приватній купі.

#include <windows.h>
#include <stdio.h>

// ======================================================
// Черга на приватній купі
// ======================================================

struct Node
{
    int   value;    // корисні дані
    Node* next;     // наступний вузол
};

class HeapQueue
{
public:
    HeapQueue()
    {
        // Приватна купа: dwFlags=0, початковий і максимальний розмір=0 (за замовчуванням,
        // тобто купа росте динамічно). HEAP_NO_SERIALIZE не використовуємо — доступ із одного потоку.
        hHeap = HeapCreate(0, 0, 0);
        if (!hHeap)
            printf("[ERROR] HeapCreate failed: %lu\n", GetLastError());
        head = tail = nullptr;
        count = 0;
        allocBytes = 0;
    }

    ~HeapQueue()
    {
        // HeapDestroy звільняє ВСЮ купу одразу — окремий HeapFree для кожного вузла не потрібен.
        if (hHeap) HeapDestroy(hHeap);
    }

    // Додати елемент у хвіст
    bool enqueue(int v)
    {
        Node* n = (Node*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(Node));
        if (!n)
        {
            printf("[ERROR] HeapAlloc failed: %lu\n", GetLastError());
            return false;
        }
        n->value = v;
        n->next = nullptr;

        if (tail)
            tail->next = n;
        else
            head = n;
        tail = n;

        count++;
        allocBytes += HeapSize(hHeap, 0, n);   // фактичний розмір блоку, виділеного купою
        return true;
    }

    // Зняти елемент з голови
    bool dequeue(int& out)
    {
        if (!head) return false;

        Node* n = head;
        out = n->value;
        head = head->next;
        if (!head) tail = nullptr;

        allocBytes -= HeapSize(hHeap, 0, n);
        HeapFree(hHeap, 0, n);
        count--;
        return true;
    }

    bool front(int& out) const
    {
        if (!head) return false;
        out = head->value;
        return true;
    }

    bool isEmpty() const { return head == nullptr; }
    size_t size() const { return count; }
    SIZE_T bytes() const { return allocBytes; }

    // Очистити чергу (звільнити кожен вузол окремо через HeapFree)
    void clear()
    {
        int tmp;
        while (dequeue(tmp)) { /* пусто */ }
    }

private:
    HANDLE hHeap;
    Node* head;
    Node* tail;
    size_t count;
    SIZE_T allocBytes;
};

// ======================================================
// Програма перевірки
// ======================================================

static void printSep(const char* title)
{
    printf("\n=========================================================\n");
    printf("%s\n", title);
    printf("=========================================================\n");
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printSep("| Завдання 2 - Черга (FIFO) на функціях купи          |");

    HeapQueue q;

    // 1. Додавання елементів
    printf("\n--- 1. Додавання елементів (enqueue) --------\n");
    int data[] = { 10, 20, 30, 40, 50 };
    for (int v : data)
    {
        q.enqueue(v);
        printf("  enqueue(%d)  -> розмір черги = %zu, виділено в купі = %llu байт\n",
            v, q.size(), (unsigned long long)q.bytes());
    }

    // 2. Перегляд голови
    printf("\n--- 2. Поточний стан ------------------------\n");
    int f = 0;
    if (q.front(f)) printf("  Голова черги (front): %d\n", f);
    printf("  Кількість елементів : %zu\n", q.size());

    // 3. Зняття елементів — порядок має співпадати з порядком додавання (FIFO)
    printf("\n--- 3. Зняття (dequeue), очікуємо 10..50 ----\n");
    int out;
    while (q.dequeue(out))
        printf("  dequeue() = %d  -> залишилось %zu, у купі %llu байт\n",
            out, q.size(), (unsigned long long)q.bytes());

    // 4. Перевірка порожньої черги
    printf("\n--- 4. Перевірка порожньої черги ------------\n");
    printf("  isEmpty() = %s\n", q.isEmpty() ? "true" : "false");
    printf("  dequeue() з порожньої = %s\n", q.dequeue(out) ? "успіх" : "хибно (черга порожня)");

    // 5. Повторне використання + очищення
    printf("\n--- 5. Повторне заповнення і clear() --------\n");
    for (int i = 1; i <= 3; i++) q.enqueue(i * 100);
    printf("  Після додавання 3 елементів: розмір = %zu\n", q.size());
    q.clear();
    printf("  Після clear(): розмір = %zu, isEmpty = %s\n",
        q.size(), q.isEmpty() ? "true" : "false");

    printf("\nГотово. (Купу буде звільнено через HeapDestroy у деструкторі.)\n");
    return 0;
}
