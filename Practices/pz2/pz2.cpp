#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cctype>

using namespace std;

const int MAX_SIZE = 100;       // максимальний розмір для псевдодинамічного одновимірного масиву
const int MAX_MATRIX_SIZE = 20; // максимальний розмір для псевдодинамічного двовимірного масиву

// ==========================================
// Завдання 2.1
// ==========================================

void generateArray(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100 - 50;
    }
}


void printArray(const int arr[], int n) {
    for (int i = 0; i < n; i++) {
        cout << arr[i] << " ";
    }
    cout << "\n\n";
}


void deleteOddIndices(int arr[], int& n) {
    int new_n = 0;
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            arr[new_n] = arr[i];
            new_n++;
        }
    }
    n = new_n;
}


void addElementAtK(int arr[], int& n, int k, int value) {
    if (n >= MAX_SIZE || k < 0 || k > n) return;
    for (int i = n; i > k; i--) {
        arr[i] = arr[i - 1];
    }
    arr[k] = value;
    n++;
}


void reverseArray(int arr[], int n) {
    for (int i = 0; i < n / 2; i++) {
        int temp = arr[i];
        arr[i] = arr[n - 1 - i];
        arr[n - 1 - i] = temp;
    }
}


int searchAverageLinear(int arr[], int n, int& comparisons, int& averageValue) {
    if (n == 0) return -1;
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    averageValue = static_cast<int>(sum / n);

    comparisons = 0;
    for (int i = 0; i < n; i++) {
        comparisons++;
        if (arr[i] == averageValue) {
            return i;
        }
    }
    return -1;
}


//  сортування методом простого обміну
void sortSimpleExchange(int arr[], int n) {
    for (int i = 1; i < n; i++) {
        for (int j = n - 1; j >= i; j--) {
            if (arr[j] < arr[j - 1]) {
                int r = arr[j];
                arr[j] = arr[j - 1];
                arr[j - 1] = r;
            }
        }
    }
}


void runTask2_1() {
    int mas[MAX_SIZE];
    int k, value;
    int n;
    int index;
    int comparisons = 0, avgValue = 0;


    cout << "\n=== ЗАВДАННЯ 2.1 ===\n"; // для краси

    // задання розміру масиву
    cout << "Введіть розмір масиву < " << MAX_SIZE << ": ";
    cin >> n;

    // перевірка чи задане значення коректне
    if (n <= 0 || n >= MAX_SIZE) {
        cout << "Некоректний розмір!\n";
        return;
    }

    // наповнюємо масив випадковими числами
    generateArray(mas, n);
    cout << "Початковий масив:\n";
    printArray(mas, n);


    // видаляємо елементи з непарними індексами
    deleteOddIndices(mas, n);
    cout << "Після видалення елементів з непарними індексами:\n";
    printArray(mas, n);

    
    // інпути
    cout << "Введіть індекс K для вставки (0 - " << n << "): ";
    cin >> k;
    
    cout << "Введіть значення: ";
    cin >> value;
    
    // додаємо value на індекс k
    addElementAtK(mas, n, k, value);
    cout << "Після додавання елемента:\n";
    printArray(mas, n);

    // перевертаємо масив
    reverseArray(mas, n);
    cout << "Після перевороту масиву:\n";
    printArray(mas, n);

    // шукаємо середнє значенн та його індекс в масиві
    index = searchAverageLinear(mas, n, comparisons, avgValue);

    cout << "Лінійний пошук середнього (" << avgValue << "):\n";
    if (index != -1) {
        cout << "Знайдено на індексі " << index << " (Порівнянь: " << comparisons << ")\n\n";
    }
    else {
        cout << "Не знайдено (Порівнянь: " << comparisons << ")\n\n";
    }

    // сортуємо масив
    sortSimpleExchange(mas, n);
    cout << "Після сортування (простий обмін):\n";
    printArray(mas, n);

    comparisons = 0, avgValue = 0; // скидаємо для коректності поторного пошуку

    // шукаємо середнє значення та його індекс у відсортованому масиві
    index = searchAverageLinear(mas, n, avgValue, comparisons);

    cout << "Лінійний пошук середнього (" << avgValue << "):\n";
    if (index != -1) {
        cout << "Знайдено на індексі " << index << " (Порівнянь: " << comparisons << ")\n\n";
    }
    else {
        cout << "Не знайдено (Порівнянь: " << comparisons << ")\n\n";
    }
}


// ==========================================
// Завдання 2.2
// ==========================================

void generateDoubleArray(double arr[], int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = (rand() % 1000 - 500) / 10.0;
    }
}


void printDoubleArray(const double arr[], int n) {
    for (int i = 0; i < n; i++) {
        cout << arr[i] << " ";
    }
    cout << "\n\n";
}


void replaceFirstMaxWithZero(double arr[], int n) {
    if (n == 0) return;

    double max_val = arr[0];
    int max_index = 0;

    for (int i = 1; i < n; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
            max_index = i;
        }
    }

    cout << "Перший максимальний елемент: " << max_val << " на індексі " << max_index << "\n";
    arr[max_index] = 0.0;
}


void runTask2_2() {
    double mas[MAX_SIZE];
    int n;

    cout << "\n=== ЗАВДАННЯ 2.2 ===\n";

    cout << "Введіть розмір масиву дійсних чисел < " << MAX_SIZE << ": ";
    cin >> n;

    if (n <= 0 || n >= MAX_SIZE) {
        cout << "Некоректний розмір!\n";
        return;
    }

    generateDoubleArray(mas, n);
    cout << "Початковий масив дійсних чисел:\n";
    printDoubleArray(mas, n);

    replaceFirstMaxWithZero(mas, n);

    cout << "Масив після заміни максимального елемента нулем:\n";
    printDoubleArray(mas, n);
}


// ==========================================
// Завдання 2.3
// ==========================================

void generateStaticMatrix(int matr[][MAX_MATRIX_SIZE], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matr[i][j] = rand() % 100 - 50;
        }
    }
}


void printStaticMatrix(const int matr[][MAX_MATRIX_SIZE], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << matr[i][j] << "\t";
        }
        cout << "\n";
    }
    cout << "\n";
}


void sumPositiveOddMainDiagonal(const int matr[][MAX_MATRIX_SIZE], int n) {
    int sum = 0;
    bool found = false;

    cout << "Елементи головної діагоналі, що задовольняють умову (позитивні і непарні): ";
    for (int i = 0; i < n; i++) {
        int elem = matr[i][i];
        if (elem > 0 && elem % 2 != 0) {
            cout << elem << " ";
            sum += elem;
            found = true;
        }
    }
    cout << "\n";

    if (found) {
        cout << "Сума позитивних непарних чисел головної діагоналі: " << sum << "\n\n";
    }
    else {
        cout << "Таких елементів на головній діагоналі немає.\n\n";
    }
}


void runTask2_3() {
    int matr[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];
    int n;

    cout << "\n=== ЗАВДАННЯ 2.3 ===\n";

    cout << "Введіть розмір квадратної матриці (N x N), де N < " << MAX_MATRIX_SIZE << ": ";
    cin >> n;

    if (n <= 0 || n >= MAX_MATRIX_SIZE) {
        cout << "Некоректний розмір!\n";
        return;
    }

    generateStaticMatrix(matr, n);
    cout << "Згенерована матриця:\n";
    printStaticMatrix(matr, n);

    sumPositiveOddMainDiagonal(matr, n);
}


// ==========================================
// Завдання 2.4 
// ==========================================

// Одновимірний масив
int* form_mas(int n) {
    return new int[n];
}

void init_mas(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100 - 50;
    }
}

void print_mas(const int* arr, int n) {
    if (n == 0 || arr == nullptr) {
        cout << "[Масив порожній]\n";
        return;
    }
    for (int i = 0; i < n; i++) {
        cout << arr[i] << " ";
    }
    cout << "\n";
}

void del_mas(int*& arr, int& n) {

    // перевірка чи не порочній масив 
    if (n <= 0 || arr == nullptr) {
        cout << "Помилка: масив порожній! Видалення неможливе.\n";
        return;
    }

    int new_n = n / 2;

    // якщо масив містить один елемент
    if (new_n == 0) {
        delete[] arr;
        arr = nullptr;
        n = 0;
        cout << "Всі елементи було видалено. Масив тепер порожній.\n";
        return;
    }


    int* new_arr = new int[new_n];
    int j = 0;

    for (int i = 0; i < n; i++) {
        if (i % 2 != 0) {
            new_arr[j++] = arr[i];
        }
    }

    delete[] arr;
    arr = new_arr;
    n = new_n;
    cout << "Елементи з парними індексами успішно видалено.\n";
}

// /|\/|\/|\/|\/|\/|\/|\/|\/|\/|\/|\/|\/|\/|\   трішки слобажанських мотивів :] 
// |    Функції для двовимірного масиву     |   
// \|/\|/\|/\|/\|/\|/\|/\|/\|/\|/\|/\|/\|/\|/
int** form_matr(int rows, int cols) {
    int** matr = new int* [rows];
    for (int i = 0; i < rows; i++) {
        matr[i] = new int[cols];
    }
    return matr;
}

void init_matr(int** matr, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matr[i][j] = rand() % 100 - 50;
        }
    }
}

void print_matr(int** matr, int rows, int cols) {
    if (rows == 0 || cols == 0 || matr == nullptr) {
        cout << "[Матриця порожня]\n";
        return;
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            cout << matr[i][j] << "\t";
        }
        cout << "\n";
    }
}

void free_matr(int** matr, int rows) {
    if (matr == nullptr) return;
    for (int i = 0; i < rows; i++) {
        delete[] matr[i];
    }
    delete[] matr;
}

void add_matr(int**& matr, int rows, int& cols, int k) {
    // валідація
    if (rows <= 0 || cols <= 0 || matr == nullptr) {
        cout << "Помилка: матриця порожня! Спочатку сформуйте її.\n";
        return;
    }
    if (k <= 0) {
        cout << "Помилка: кількість стовпців для додавання має бути більшою за 0!\n";
        return;
    }

    int new_cols = cols + k;
    int** new_matr = form_matr(rows, new_cols);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            new_matr[i][j] = matr[i][j];
        }
        for (int j = cols; j < new_cols; j++) {
            new_matr[i][j] = 0;
        }
    }

    free_matr(matr, rows);
    matr = new_matr;
    cols = new_cols;
    cout << k << " стовпців успішно додано.\n";
}


void runTask2_4() {
    int k_menu;

    int* mas = nullptr;
    int n = 0;

    int** matr = nullptr;
    int rows = 0, cols = 0;

    cout << "\n=== ЗАВДАННЯ 2.4 ===\n";

    do {
        cout << "\n1. Формування масиву\n";
        cout << "2. Друк масиву\n";
        cout << "3. Видалення з масиву (1D - парні індекси)\n";
        cout << "4. Додавання в масив (2D - стовпці в кінець)\n";
        cout << "5. Вихід\n";
        cout << "Вибір: ";
        cin >> k_menu;

        switch (k_menu) {
        case 1: // формування масиву 

            // звільняємо стару пам'ять, якщо формуємо наново
            if (mas != nullptr) { 
                delete[] mas; 
                mas = nullptr; 
                n = 0; 
            }
            if (matr != nullptr) { 
                free_matr(matr, rows); 
                matr = nullptr; 
                rows = 0; 
                cols = 0; 
            }

            // формуємо одновимірний масив
            cout << "Введіть розмір одновимірного масиву: ";
            cin >> n;
            if (n > 0) {
                mas = form_mas(n);
                init_mas(mas, n);
            }


            // формування двовиміного масиву
            cout << "Введіть кількість стовпців 2D матриці: ";
            cin >> cols;

            cout << "Введіть кількість рядків 2D матриці: ";
            cin >> rows;

            // валідація та формування матриці
            if (rows > 0 && cols > 0) {
                matr = form_matr(rows, cols);
                init_matr(matr, rows, cols);
            }
            break;
        case 2: // виведення масиву
            cout << "\n--- 1D Масив ---\n";
            print_mas(mas, n);
            cout << "\n--- 2D Матриця ---\n";
            print_matr(matr, rows, cols);
            break;
        case 3: // видалення масиву
            del_mas(mas, n);
            break;
        case 4: // додавання стовпців
            int k_cols;

            cout << "Скільки стовпців додати в кінець? ";
            cin >> k_cols;
            add_matr(matr, rows, cols, k_cols);
            break;
        case 5: // вихід
            // очищуємо пам'ять
            if (mas != nullptr) delete[] mas;
            if (matr != nullptr) free_matr(matr, rows);
            cout << "Повернення до головного меню...\n";
            break;
        default:
            cout << "Невірний вибір!\n";
        }
    } while (k_menu != 5);
}


// ==========================================
// Завдання 2.5
// ==========================================

bool isVowel(char c) {
    c = tolower(c); // приводимо до нижнього регістру для коректності

    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y' ||  // англійські
        c == 'а' || c == 'е' || c == 'и' || c == 'і' || c == 'ї' || c == 'о' ||
        c == 'у' || c == 'ю' || c == 'я'; // українські
}


void removeVowels(string& s) {
    string result = "";

    for (char c : s) {
        if (!isVowel(c)) {
            result += c;
        }
    }

    s = result;
}


// перевірка, чи може рядок бути шістнадцятковим числом
bool isHexWord(const string& word) {
    if (word.empty()) return false; // якщо слово пусте то вертаємо false

    for (char c : word) { // проходимся по кожному символу в слові 
        // isxdigit перевіряє чи є символ 0-9, a-f, A-F
        if (!isxdigit(c)) return false;
    }
    return true;
}


void processHexString(string& s) {
    // виділення 16-річних констант та їх перемноження
    unsigned long long product = 1;
    bool foundHex = false; // флажок чи був hex в рядку
    string currentWord = "";

    // приводимо всі символи рядку до верхнього регістру 
    for (char& c : s) {
        c = toupper(c);
    }

    cout << "Рядок у верхньому регістрі: " << s << "\n";

    // додаємо пробіл в кінець, щоб обробити останнє слово
    s += " ";

    // прохдимось по кожному символу тексту
    for (char c : s) {


        // якщо символ не пробіл то збираємо слово 
        if (!isspace(c)) {
            currentWord += c;
            continue;
        }

        // на випадок якщо не один пробіл, то переходимо до наступного
        if (currentWord.empty()) continue;


        // перевіряємо чи загалом це може бути hex чи ні
        if (isHexWord(currentWord)) {

            // переводимо за допомогою stoull - string to unsigned long long (base 16)
            unsigned long long decimalValue = stoull(currentWord, nullptr, 16);

            cout << "Знайдено 16-річну константу: "
                 << currentWord
                 << " (у десятковій: "
                 << decimalValue << ")\n";

            
            product *= decimalValue; // множимо число для отримання добутку всіх чисел в рядку
            foundHex = true;
        }

        currentWord.clear(); 
    }



    if (foundHex) {
        cout << "Добуток знайдених 16-річних чисел (у десятковій системі): " << product << "\n";
    }
    else {
        cout << "У рядку не знайдено 16-річних констант.\n";
    }
}

void runTask2_5() {
    string text1, text2;

    cin.ignore(); // очищуємо буфер після попередніх cin
    cout << "\n=== ЗАВДАННЯ 2.5 ===\n";


    cout << "\n--- ЗАВДАННЯ 2.5.1 (Видалення голосних) ---\n";
    cout << "Введіть рядок: ";
    getline(cin, text1); // використовуємо getline для зчитування рядка з пробілами

    removeVowels(text1);
    cout << "Рядок без голосних літер: " << text1 << "\n";

    cout << "\n--- ЗАВДАННЯ 2.5.2 (16-річні константи) ---\n";
    cout << "Введіть стрингову константу (рядок): ";
    getline(cin, text2);

    processHexString(text2);
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

int main() {
    srand(static_cast<unsigned int>(time(0)));
    int choice;

    do {
        cout << "===========================\n";
        cout << "МЕНЮ ЛАБОРАТОРНОЇ РОБОТИ\n";
        cout << "1. Запустити Завдання 2.1 (Псевдодинамічні масиви)\n";
        cout << "2. Запустити Завдання 2.2 (Вставка / Заміна)\n";
        cout << "3. Запустити Завдання 2.3 (Матриці - статика)\n";
        cout << "4. Запустити Завдання 2.4 (Динамічні масиви)\n";
        cout << "5. Запустити Завдання 2.5 (Рядки)\n";
        cout << "0. Вихід\n";
        cout << "===========================\n";
        cout << "Ваш вибір: ";
        cin >> choice;

        switch (choice) {
        case 1: runTask2_1(); break;
        case 2: runTask2_2(); break;
        case 3: runTask2_3(); break;
        case 4: runTask2_4(); break;
        case 5: runTask2_5(); break;
        case 0: cout << "Вихід...\n"; break;
        default: cout << "Невірний вибір!\n";
        }
    } while (choice != 0);

    return 0;
}

//                     ____________________  
//                    /                    \
//                    |  Pochita is here!  |
//                    \__  ________________/
//                       |/
//  ⠀⠀⠀⠀⣀⠀⠀⠀⠀⠀⠀⣠⣶⣶⣶⣶⣶⣶⣤⡀⠀⠀⠀⠀     
//  ⠰⣤⣾⣾⣿⣤⣠⣖⣀⢀⣰⣿⠋⠀⠀⢀⣾⡿⠿⢷⣀⠀⠀⠀     
//  ⢰⣿⡟⠏⠙⠛⢻⡿⡿⠟⠉⠉⠁⠉⠉⣿⡿⠓⠤⢀⣹⠀⠀⠀     
//  ⣨⣿⣷⣮⠀⠄⠄⡔⢁⣭⣤⡀⠀⠀⠀⠈⠀⠀⠀⠀⠈⠢⣀⠀   
//  ⠀⠘⠛⣿⢷⣶⣴⠀⢸⠈⡿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣷      
//  ⠀⠀⠀⠁⠀⢠⢃⣀⠁⠘⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇
//  ⠀⠀⠀⠀⠀⠘⣝⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠇
//  ⠀⠀⠀⠀⠀⠀⠈⠢⢤⠀⠀⢀⠀⠀⠀⠀⢰⠀⣰⡀⠀⠀⠃⠀
//  ⠀⠀⠀⠀⠀⠀⠀⠀⠈⣊⣸⡿⠉⠑⡦⠀⣸⡟⠀⠙⡦⠀⡇⠀
//  ⠀⠀⠀⠀⠀⠀⠀⠀⠘⠛⠋⠀⠀⠸⠦⠚⠉⠀⠀⠀⠃⠛⠀⠀