#include <windows.h>
#include <stdio.h>


// черга на приватній купі
struct Node
{
    int   value;    // дані
    Node* next;     // вказівник на наступний
};

class HeapQueue
{
public:
    HeapQueue()
    {
        // приватна купа: dwFlags=0, старт/макс=0 (динамічне розширення).
        // HEAP_NO_SERIALIZE не використовується — доступ лише з одного потоку
        hHeap = HeapCreate(0, 0, 0);
        if (!hHeap)
            printf("[ERROR] HeapCreate failed: %lu\n", GetLastError());
        head = tail = nullptr;
        count = 0;
        allocBytes = 0;
    }

    ~HeapQueue()
    {
        // heapdestroy звільняє всю купу одразу — окремі heapfree для вузлів не потрібні
        if (hHeap) HeapDestroy(hHeap);
    }

    // додання елементу у хвіст
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
        allocBytes += HeapSize(hHeap, 0, n);   // фактичний розмір блоку в купі
        return true;
    }

    // знятитя елементу з голови
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

    // очистка черги
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


// тестування


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

    // 1. додавання елементів
    printf("\n--- 1. Додавання елементів (enqueue) --------\n");
    int data[] = { 10, 20, 30, 40, 50 };
    for (int v : data)
    {
        q.enqueue(v);
        printf("  enqueue(%d)  -> розмір черги = %zu, виділено в купі = %llu байт\n",
            v, q.size(), (unsigned long long)q.bytes());
    }

    // 2. перегляд голови
    printf("\n--- 2. Поточний стан ------------------------\n");
    int f = 0;
    if (q.front(f)) printf("  Голова черги (front): %d\n", f);
    printf("  Кількість елементів : %zu\n", q.size());

    // 3. зняття елементів — порядок має співпадати з порядком додавання (FIFO)
    printf("\n--- 3. Зняття (dequeue), очікуємо 10..50 ----\n");
    int out;
    while (q.dequeue(out))
        printf("  dequeue() = %d  -> залишилось %zu, у купі %llu байт\n",
            out, q.size(), (unsigned long long)q.bytes());

    // 4. перевірка порожньої черги
    printf("\n--- 4. Перевірка порожньої черги ------------\n");
    printf("  isEmpty() = %s\n", q.isEmpty() ? "true" : "false");
    printf("  dequeue() з порожньої = %s\n", q.dequeue(out) ? "успіх" : "хибно (черга порожня)");

    // 5. повторне використання + очищення
    printf("\n--- 5. Повторне заповнення і clear() --------\n");
    for (int i = 1; i <= 3; i++) q.enqueue(i * 100);
    printf("  Після додавання 3 елементів: розмір = %zu\n", q.size());
    q.clear();
    printf("  Після clear(): розмір = %zu, isEmpty = %s\n",
        q.size(), q.isEmpty() ? "true" : "false");

    printf("\nГотово. (Купу буде звільнено через HeapDestroy у деструкторі.)\n");
    return 0;
}
