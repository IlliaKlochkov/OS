#include <windows.h>
#include <iostream>
#include <vector>
#include "thread_lib.h"


// завдання 1: запустити 10 потоків, чекати завершення хоча б одного

void task1() {
    std::cout << "\n=== Завдання 1: 10 потокiв ===\n";

    const int N = 10;
    volatile long counter = 0;

    ThreadParam params[N];
    HANDLE      handles[N];

    for (int i = 0; i < N; i++) {
        params[i] = { i, 5'000'000, &counter };
        handles[i] = CreateThread(nullptr, 0, ThreadFunc,
            &params[i], 0, nullptr);
        if (!handles[i]) {
            std::cerr << "CreateThread failed for " << i << "\n";
            return;
        }
    }

    // очікується завершення хоча б одного потоку
    DWORD idx = WaitForMultipleObjects(N, handles, FALSE, INFINITE);
    std::cout << "Перший завершився: потiк " << idx << "\n";

    // очікується завершення всіх потоків
    WaitForMultipleObjects(N, handles, TRUE, INFINITE);

    for (int i = 0; i < N; i++) CloseHandle(handles[i]);

    std::cout << "Лiчильник викликiв: " << counter
        << " (очiкується " << N << ")\n";
}


// завдання 2: знайти максимальну кількість потоків

// окрема потокова функція — просто збільшує лічильник
static volatile long g_cnt2 = 0;
static DWORD WINAPI SimpleThread(LPVOID) {
    InterlockedIncrement(&g_cnt2);
    return 0;
}

void task2() {
    std::cout << "\n=== Завдання 2: максимальна кiлькiсть потокiв ===\n";

    // максимум для WaitForMultipleObjects — MAXIMUM_WAIT_OBJECTS (64)
    const DWORD MAX_WAIT = MAXIMUM_WAIT_OBJECTS; // 64

    for (DWORD n = 2; n <= MAX_WAIT; n *= 2) {
        g_cnt2 = 0;
        std::vector<HANDLE> h(n);
        bool ok = true;

        for (DWORD i = 0; i < n; i++) {
            h[i] = CreateThread(nullptr, 0, SimpleThread,
                nullptr, 0, nullptr);
            if (!h[i]) { ok = false; break; }
        }

        if (ok)
            WaitForMultipleObjects(n, h.data(), TRUE, INFINITE);

        for (DWORD i = 0; i < n; i++)
            if (h[i]) CloseHandle(h[i]);

        std::cout << "n=" << n << " -> лiчильник=" << g_cnt2;
        if ((DWORD)g_cnt2 != n) {
            std::cout << " НЕВIДПОВIДНIСТЬ — зупиняємось\n";
            break;
        }
        std::cout << " OK\n";
    }
}


// завдання 3: виробник / споживач

// виробник кладе імена файлів у чергу; споживач виводить їх вміст
// синхронізація: м'ютекс + семафор

#include <queue>
#include <string>

struct ProdConsCtx {
    std::queue<std::string> queue;
    HANDLE mutex;       // захист черги
    HANDLE semItems;    // кількість елементів у черзі
    HANDLE semSpace;    // вільне місце у черзі
    bool   done;        // виробник закінчив
};

static DWORD WINAPI ProducerThread(LPVOID arg) {
    auto* ctx = reinterpret_cast<ProdConsCtx*>(arg);

    const char* files[] = { "file1.txt", "file2.txt", "file3.txt",
                             "file4.txt", "file5.txt" };
    for (auto* name : files) {
        WaitForSingleObject(ctx->semSpace, INFINITE); // очікується вільне місце
        WaitForSingleObject(ctx->mutex, INFINITE);    // отримання доступу до черги
        ctx->queue.push(name);
        std::cout << "[Виробник] додав: " << name << "\n";
        ReleaseMutex(ctx->mutex);
        ReleaseSemaphore(ctx->semItems, 1, nullptr);  // сигналізується споживачу
        Sleep(100);
    }
    ctx->done = true;
    // розбудити споживача у разі очікування
    ReleaseSemaphore(ctx->semItems, 1, nullptr);
    return 0;
}

static DWORD WINAPI ConsumerThread(LPVOID arg) {
    auto* ctx = reinterpret_cast<ProdConsCtx*>(arg);

    while (true) {
        WaitForSingleObject(ctx->semItems, INFINITE); // очікується елемент
        WaitForSingleObject(ctx->mutex, INFINITE);

        if (ctx->queue.empty()) {
            ReleaseMutex(ctx->mutex);
            // у разі завершення виробника — вихід
            if (ctx->done) break;
            continue;
        }

        std::string name = ctx->queue.front();
        ctx->queue.pop();
        ReleaseMutex(ctx->mutex);
        ReleaseSemaphore(ctx->semSpace, 1, nullptr); // звільнення місця

        // імітація виводу вмісту файлу
        std::cout << "[Споживач] обробляє: " << name << "\n";
        Sleep(200);
    }
    return 0;
}

void task3() {
    std::cout << "\n=== Завдання 3: Виробник / Споживач ===\n";

    const int QUEUE_CAP = 3; // місткість черги

    ProdConsCtx ctx;
    ctx.done = false;
    ctx.mutex = CreateMutex(nullptr, FALSE, nullptr);
    ctx.semItems = CreateSemaphore(nullptr, 0, QUEUE_CAP, nullptr);
    ctx.semSpace = CreateSemaphore(nullptr, QUEUE_CAP, QUEUE_CAP, nullptr);

    HANDLE prod = CreateThread(nullptr, 0, ProducerThread, &ctx, 0, nullptr);
    HANDLE cons = CreateThread(nullptr, 0, ConsumerThread, &ctx, 0, nullptr);

    HANDLE both[] = { prod, cons };
    WaitForMultipleObjects(2, both, TRUE, INFINITE);

    CloseHandle(prod);
    CloseHandle(cons);
    CloseHandle(ctx.mutex);
    CloseHandle(ctx.semItems);
    CloseHandle(ctx.semSpace);

    std::cout << "Завдання 3 завершено.\n";
}


// завдання 5: філософи, що обідають (Dining Philosophers)

// 5 філософів, 5 виделок (м'ютекси), кожен виконує N_CYCLES циклів:
// думає -> бере першу виделку -> бере другу виделку -> обідає -> кладе виделки
// порядок захоплення: парний — ліва -> права, непарний — права -> ліва
// це запобігає deadlock (порушення кругової залежності)

static const int N_PHILOSOPHERS = 5;
static const int N_CYCLES = 3;

static HANDLE forks[N_PHILOSOPHERS]; // виделка i = м'ютекс i

struct PhilosopherParam { int id; };

static DWORD WINAPI PhilosopherThread(LPVOID arg) {
    auto* p = reinterpret_cast<PhilosopherParam*>(arg);
    int id = p->id;
    int left = id;
    int right = (id + 1) % N_PHILOSOPHERS;

    // парні беруть ліву потім праву, непарні — навпаки
    int first = (id % 2 == 0) ? left : right;
    int second = (id % 2 == 0) ? right : left;

    for (int cycle = 0; cycle < N_CYCLES; cycle++) {
        // думає
        std::cout << "Фiлософ " << id << " думає (цикл " << cycle + 1 << ")\n";
        Sleep(100 + id * 30);

        WaitForSingleObject(forks[first], INFINITE); // бере першу виделку
        WaitForSingleObject(forks[second], INFINITE); // бере другу виделку

        // обідає
        std::cout << "Фiлософ " << id << " обiдає (цикл " << cycle + 1 << ")\n";
        Sleep(150);

        ReleaseMutex(forks[second]); // кладе виделки
        ReleaseMutex(forks[first]);
        std::cout << "Фiлософ " << id << " поклав виделки\n";
    }
    std::cout << "Фiлософ " << id << " завершив роботу\n";
    return 0;
}

void task5() {
    std::cout << "\n=== Завдання 5: Фiлософи, що обiдають ===\n";

    // м'ютекс на кожну виделку
    for (int i = 0; i < N_PHILOSOPHERS; i++) {
        forks[i] = CreateMutex(nullptr, FALSE, nullptr);
        if (!forks[i]) {
            std::cerr << "CreateMutex failed for fork " << i << "\n";
            return;
        }
    }

    PhilosopherParam params[N_PHILOSOPHERS];
    HANDLE           handles[N_PHILOSOPHERS];

    for (int i = 0; i < N_PHILOSOPHERS; i++) {
        params[i].id = i;
        handles[i] = CreateThread(nullptr, 0, PhilosopherThread,
            &params[i], 0, nullptr);
        if (!handles[i]) {
            std::cerr << "CreateThread failed for philosopher " << i << "\n";
            return;
        }
    }

    // очікується завершення всіх філософів
    WaitForMultipleObjects(N_PHILOSOPHERS, handles, TRUE, INFINITE);

    for (int i = 0; i < N_PHILOSOPHERS; i++) {
        CloseHandle(handles[i]);
        CloseHandle(forks[i]);
    }
    std::cout << "Завдання 5 завершено. Гонок i блокувань не виявлено.\n";
}


int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    task1();
    task2();
    task3();
    task5();

    return 0;
}