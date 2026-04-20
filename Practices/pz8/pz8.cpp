#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <windows.h>

using namespace std;

// ==========================================
// Завдання 1
// ==========================================

// менше значення = вищий пріоритет ( 0 - найвищий)
struct ProcessT1 {
    int id;
    int remaining_time;
    int initial_priority;
    int current_priority;

    // поля для статистики
    int arrival_time;
    int first_start_time;
    int completion_time;
};

void runTask1() {
    // ініціалізуємо список процесів 
    // (ID, Час виконання, Початковий пріор, Поточний пріор, Arrival, First Start, Completion)
    vector<ProcessT1> processes = {
        {1, 5, 2, 2, 0, -1, 0},
        {2, 3, 3, 3, 0, -1, 0},
        {3, 4, 1, 1, 0, -1, 0},
        {4, 2, 4, 4, 0, -1, 0}
    };

    int current_time = 0;
    int completed_processes = 0;
    int n = processes.size();

    // Метрики
    int context_switches = 0;
    int last_executed_id = -1;

    printf("> Початковий стан процесів:\n");
    for (const auto& p : processes) {
        printf("> Процес P%d: Час = %d, Початковий пріоритет = %d\n", p.id, p.remaining_time, p.initial_priority);
    }
    printf("--------------------------------------------------\n");

    // виконуємо квантування (квант часу = 1)
    while (completed_processes < n) {
        int highest_priority_idx = -1;
        int highest_priority = 9999;

        // шукаємо процес із найвищим пріоритетом
        for (int i = 0; i < n; i++) {
            if (processes[i].remaining_time > 0) {
                if (processes[i].current_priority < highest_priority) {
                    highest_priority = processes[i].current_priority;
                    highest_priority_idx = i;
                }
            }
        }

        if (highest_priority_idx != -1) {
            // відстеження переключень контексту
            if (last_executed_id != processes[highest_priority_idx].id && last_executed_id != -1) {
                context_switches++;
            }
            last_executed_id = processes[highest_priority_idx].id;

            // Відстеження першого запуску
            if (processes[highest_priority_idx].first_start_time == -1) {
                processes[highest_priority_idx].first_start_time = current_time;
            }

            // процес отримує процесорний час
            processes[highest_priority_idx].remaining_time--;
            printf(
                "[INFO] Час %d: Виконується P%d (Залишилось: %d, Пріоритет: %d)\n",
                current_time, 
                processes[highest_priority_idx].id,
                processes[highest_priority_idx].remaining_time, 
                processes[highest_priority_idx].current_priority
            );

            // оновлюємо пріоритети для всіх процесів
            for (int i = 0; i < n; i++) {
                if (processes[i].remaining_time > 0) {
                    if (i == highest_priority_idx) {
                        processes[i].current_priority = processes[i].initial_priority;
                    }
                    else {
                        processes[i].current_priority--;
                    }
                }
            }

            // перевіряємо, чи завершився процес
            if (processes[highest_priority_idx].remaining_time == 0) {
                completed_processes++;
                processes[highest_priority_idx].completion_time = current_time + 1; // Час після завершення кванту
                printf("[INFO] >>> Процес P%d ЗАВЕРШЕНО <<<\n", processes[highest_priority_idx].id);
            }
        }
        current_time++;
    }

    // --- обчислення та вивід статистики ---
    double total_turnaround = 0;
    double total_wait_for_start = 0;

    for (const auto& p : processes) {
        total_turnaround += (p.completion_time - p.arrival_time);
        total_wait_for_start += (p.first_start_time - p.arrival_time);
    }

    printf("--------------------------------------------------\n");
    printf("Кількість переключень між процесами\t%d\n", context_switches);
    printf("Середній час виконання\t\t\t%.1f\n", total_turnaround / n);
    printf("Середній час чекання запуску\t\t%.1f\n", total_wait_for_start / n);
}


// ==========================================
// Завдання 2
// ==========================================
struct ProcessT2 {
    int id;
    int remaining_time;

    // поля для статистики
    int arrival_time;
    int first_start_time;
    int completion_time;
};

/*
універсальна функція для обробки процесів у MLFQ
next_q передається як вказівник, щоб можна було передати nullptr для останньої черги
*/
void processQueue(queue<ProcessT2>& current_q, queue<ProcessT2>* next_q, int quantum,
    int& current_time, int& context_switches, int& last_executed_id,
    vector<ProcessT2>& finished_processes, const string& q_name)
{
    ProcessT2 p = current_q.front();
    current_q.pop();

    if (last_executed_id != p.id && last_executed_id != -1) context_switches++;
    last_executed_id = p.id;

    if (p.first_start_time == -1) p.first_start_time = current_time;

    // Якщо quantum == -1, це означає нескінченність (для Q4)
    int execution_time = (quantum == -1) ? p.remaining_time : min(p.remaining_time, quantum);
    p.remaining_time -= execution_time;
    current_time += execution_time;

    if (quantum == -1) {
        printf("[INFO] Час %d: %s виконує P%d до повного завершення (залишилось: 0)\n", current_time, q_name.c_str(), p.id);
    }
    else {
        printf("[INFO] Час %d: %s виконує P%d (виділено часу: %d, залишилось: %d)\n", current_time, q_name.c_str(), p.id, execution_time, p.remaining_time);
    }

    // gеревірка, чи процес не завершився і чи є куди його переводити
    if (p.remaining_time > 0 && next_q != nullptr) {
        next_q->push(p);
    }
    else {
        p.completion_time = current_time;
        finished_processes.push_back(p);
        printf("[INFO] >>> Процес P%d ЗАВЕРШЕНО <<<\n", p.id);
    }
}


void runTask2() {
    queue<ProcessT2> Q1, Q2, Q3, Q4;
    vector<ProcessT2> finished_processes; // для збереження результатів та виводу

    const int tq1 = 2;
    const int tq2 = 4;
    const int tq3 = 8;

    // після запуску процеси записуються в Q1 (Arrival = 0, First Start = -1)
    Q1.push({ 1, 5, 0, -1, 0 });
    Q1.push({ 2, 15, 0, -1, 0 });
    Q1.push({ 3, 8, 0, -1, 0 });
    Q1.push({ 4, 3, 0, -1, 0 });

    int total_processes = Q1.size();
    printf("Кванти часу: Q1 = %d, Q2 = %d, Q3 = %d, Q4 = Нескінченність\n", tq1, tq2, tq3);
    printf("Всі процеси спочатку додані до Q1.\n");
    printf("--------------------------------------------------\n");

    int current_time = 0;
    int context_switches = 0;
    int last_executed_id = -1;

    while (!Q1.empty() || !Q2.empty() || !Q3.empty() || !Q4.empty()) {
        if (!Q1.empty()) {
            processQueue(Q1, &Q2, tq1, current_time, context_switches, last_executed_id, finished_processes, "Q1");
        }
        else if (!Q2.empty()) {
            processQueue(Q2, &Q3, tq2, current_time, context_switches, last_executed_id, finished_processes, "Q2");
        }
        else if (!Q3.empty()) {
            processQueue(Q3, &Q4, tq3, current_time, context_switches, last_executed_id, finished_processes, "Q3");
        }
        else if (!Q4.empty()) {
            // передаємо null замість наступної черги та квант -1 (нескінченність)
            processQueue(Q4, nullptr, -1, current_time, context_switches, last_executed_id, finished_processes, "Q4");
        }
    }

    // --- обчислення та вивід статистики ---
    double total_turnaround = 0;
    double total_wait_for_start = 0;

    for (const auto& p : finished_processes) {
        total_turnaround += (p.completion_time - p.arrival_time);
        total_wait_for_start += (p.first_start_time - p.arrival_time);
    }

    printf("--------------------------------------------------\n");
    printf("Кількість переключень між процесами\t%d\n", context_switches);
    printf("Середній час виконання\t\t\t%.1f\n", total_turnaround / total_processes);
    printf("Середній час чекання запуску\t\t%.1f\n", total_wait_for_start / total_processes);
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

void printSeparator(const char* title)
{
    printf("\n==================================================\n");
    printf("%s\n", title);
    printf("==================================================\n");
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printSeparator("Завдання 1: Динамічний пріоритет");
    runTask1();

    printSeparator("Завдання 2: Багаторівневі черги (MLFQ)");
    runTask2();

    return 0;
}