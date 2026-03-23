#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <math.h>
#include <windows.h>
#include <cstring>

using namespace std;

const int MAX_SIZE = 100;

// просто допоміжна функція для зручного виводу
void printTaskTitle(double task_num) {
    if (task_num == (int)task_num) {
        printf("\n=== ЗАВДАННЯ %d: \n", (int)task_num);
    }
    else {
        printf("\n=== ЗАВДАННЯ %.1f: ===\n", task_num);
    }
}

// ==========================================
// Завдання 1
// ==========================================

// функція для обрахування відстані між двома точками
float calculateDistance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}


void runTask1() {
    float firstX, firstY;
    float prevX, prevY;
    float currX, currY;
    float P = 0;

    printTaskTitle(1);

    // проходимось по кожній точці (x, y) десятикутника
    for (int i = 1; i <= 10; i++) {
        cout << "Введіть x" << i << ": ";
        cin >> currX;

        cout << "Введіть y" << i << ": ";
        cin >> currY;

        cout << "\n";

        // якщо це перша точка то зберігаємо її, щоб потім замкнути фігуру
        if (i == 1) {
            firstX = currX;
            firstY = currY;
        }
        // інакше обраховуємо довжину сторони та додаємо в периметр 
        else { 
            P += calculateDistance(prevX, prevY, currX, currY); 
        }

        // наостанок робимо поточну точку минулою, для коректних наступних розрахунків
        prevX = currX;
        prevY = currY;
    }

    // в кінці обраховуємо останню сторону між першою та останньою точкою
    P += calculateDistance(prevX, prevY, firstX, firstY);

    cout << "Периметр десятикутника: " << P;
}


// ==========================================
// Завдання 2.1
// ==========================================

// заповнюємо масив випадковими числами
void fillArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 100 - 50;
    }
}

// "красивий" вивід масиву в консоль
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        // 4d - виділяємо 4 символи під елемент
        printf("%4d", arr[i]);
    }

    cout << "\n";
}

void processArray(int arr[], int& size) {
    int newSize = 0;
    
    for (int i = 0; i < size; i++) {
        if ((i + 1) % 2 != 0) {
            arr[newSize] = arr[i];
            newSize++;
        }
    }
    
    size = newSize;
}

void runTask2_1() {
    int n, arr[MAX_SIZE];

    printTaskTitle(2.1);

    cout << "Розмір масиву: ";
    cin >> n;

    fillArray(arr, n);
    cout << "Початковий масив: ";
    printArray(arr, n);

    processArray(arr, n);
    cout << "Після видалення:  ";
    printArray(arr, n);
    cout << "\n";
}

// ==========================================
// Завдання 2.2
// ==========================================

// заповнюємо матрицю рандомними числами 
void fillMatrix(int mat[MAX_SIZE][MAX_SIZE], int rows, int cols) {
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            mat[i][j] = rand() % 100 - 50;
}

// "красивий" вивід 
void printMatrix(int mat[MAX_SIZE][MAX_SIZE], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%4d", mat[i][j]);
        }
        cout << "\n";
    }
}

void sortColumnsByFirstRow(int mat[MAX_SIZE][MAX_SIZE], int rows, int cols) {
    for (int j = 0; j < cols - 1; j++) {
        for (int k = 0; k < cols - j - 1; k++) {
            if (mat[0][k] > mat[0][k + 1]) {
                for (int i = 0; i < rows; i++) {
                    int temp = mat[i][k];
                    mat[i][k] = mat[i][k + 1];
                    mat[i][k + 1] = temp;
                }
            }
        }
    }
}

void runTask2_2() {
    int r, c; // r - rows, c - columns 
    int mat[MAX_SIZE][MAX_SIZE]; // сама матриця

    printTaskTitle(2.2);

    cout << "Введіть кількість рядків: ";
    cin >> r;

    cout << "Введіть кількість стовпчиків: ";
    cin >> c;

    fillMatrix(mat, r, c);
    cout << "Початкова матриця:\n";
    printMatrix(mat, r, c);

    sortColumnsByFirstRow(mat, r, c);
    cout << "Відсортована матриця:\n";
    printMatrix(mat, r, c);
    cout << "\n";
}

// ==========================================
// Завдання 3
// ==========================================

struct Student {
    char* name;
    int kurs;
    float rating;
};

// функція для створення одного студента
Student make_student() {
    Student s;
    char temp[100];

    cout << "Ім'я студента: ";
    cin >> temp;

    s.name = new char[strlen(temp) + 1];
    strcpy(s.name, temp);

    cout << "Курс: ";
    cin >> s.kurs;

    cout << "Рейтинг: ";
    cin >> s.rating;
	cout << "\n";
    return s;
}

// функція для виводу одного студента
void print_student(const Student& s) {
    printf("Ім'я: %-10s | Курс: %d | Рейтинг: %.2f\n",
        s.name, s.kurs, s.rating);
}

// копіювання одного студента
void copyStudent(Student& dest, const Student& src) {
    dest.name = new char[strlen(src.name) + 1];
    strcpy(dest.name, src.name);
    dest.kurs = src.kurs;
    dest.rating = src.rating;
}

// звільнення пам’яті одного студента
void freeStudent(Student& s) {
    delete[] s.name;
    s.name = nullptr;
}

// заповнення динамічного масиву студентів
void fillStudentArray(Student* arr, int size) {
    for (int i = 0; i < size; i++) {
        printf("Студент №%d:\n", i + 1);
        arr[i] = make_student();
    }
}

// вивід масиву студентів
void printStudentArray(Student* arr, int size) {
    if (!arr || size <= 0) {
        cout << "Масив студентів порожній.\n";
        return;
    }

    for (int i = 0; i < size; i++) {
        print_student(arr[i]);
    }
}

// пошук студентів, у яких рейтинг менше 3
Student* findLowRatingStudents(Student* arr, int size, int& newSize) {
    newSize = 0;

    // спочатку визначаємо, скільки студентів задовольняє умову
    for (int i = 0; i < size; i++) {
        if (arr[i].rating < 3.0) {
            newSize++;
        }
    }

    // якщо таких студентів немає - повертаємо nullptr
    if (newSize == 0) {
        return nullptr;
    }

    Student* result = new Student[newSize];
    int j = 0;

    // копіюємо тільки потрібних студентів у новий масив
    for (int i = 0; i < size; i++) {
        if (arr[i].rating < 3.0) {
            copyStudent(result[j], arr[i]);
            j++;
        }
    }

    return result;
}

// звільнення пам’яті динамічного масиву студентів
void freeStudentArray(Student* arr, int size) {
    if (!arr) return;

    for (int i = 0; i < size; i++) {
        freeStudent(arr[i]);
    }

    delete[] arr;
}

// створення динамічного масиву рядків
string* createStringArray(int n) {
    if (n <= 0) {
        return nullptr;
    }

    string* arr = new string[n];

    for (int i = 0; i < n; i++) {
        cout << "Введіть рядок " << i + 1 << ": ";
        cin >> arr[i];
    }

    return arr;
}

// вивід масиву рядків
void printStringArray(string* arr, int n) {
    if (!arr || n <= 0) {
        cout << "Масив рядків порожній.\n";
        return;
    }

    for (int i = 0; i < n; i++) {
        cout << "[" << i + 1 << "]: " << arr[i] << endl;
    }
}

// видалення K рядків з кінця масиву
void deleteKRowsFromEnd(string*& arr, int& n, int k) {
    // перевірка на порожній масив
    if (!arr || n <= 0) {
        cout << "Помилка: масив рядків порожній.\n";
        return;
    }

    // K має бути додатним
    if (k <= 0) {
        cout << "Помилка: кількість рядків K має бути більшою за 0.\n";
        return;
    }

    // якщо K більше за кількість рядків
    if (k > n) {
        cout << "Помилка: кількість рядків для видалення більша за розмір масиву.\n";
        return;
    }

    int newSize = n - k;

    // якщо після видалення масив стане порожнім
    if (newSize == 0) {
        delete[] arr;
        arr = nullptr;
        n = 0;
        return;
    }

    string* newArr = new string[newSize];

    // копіюємо тільки ті рядки, які залишаються
    for (int i = 0; i < newSize; i++) {
        newArr[i] = arr[i];
    }

    delete[] arr;
    arr = newArr;
    n = newSize;
}

void runTask3() {
    int n;
    int lowRatingCount;
    int strCount;
    int k;

    printTaskTitle(3);

    // ---------------------------
    // Частина 1. Робота зі студентами
    // ---------------------------
    cout << "Кількість студентів: ";
    cin >> n;

    if (n <= 0) {
        cout << "Помилка: кількість студентів має бути більшою за 0.\n";
        return;
    }

    Student* students = new Student[n];
    fillStudentArray(students, n);

    cout << "\nПочатковий список студентів:\n";
    printStudentArray(students, n);

    Student* filtered = findLowRatingStudents(students, n, lowRatingCount);

    if (filtered == nullptr) {
        cout << "\nСтудентів з рейтингом менше 3 не знайдено.\n";
    }
    else {
        cout << "\nСтуденти з рейтингом менше 3:\n";
        printStudentArray(filtered, lowRatingCount);
    }

    // ---------------------------
    // Частина 2. Робота з масивом рядків
    // ---------------------------
    cout << "\nКількість рядків у масиві: ";
    cin >> strCount;

    string* stringArr = createStringArray(strCount);

    cout << "\nПочатковий масив рядків:\n";
    printStringArray(stringArr, strCount);

    cout << "\nСкільки рядків видалити з кінця? ";
    cin >> k;

    deleteKRowsFromEnd(stringArr, strCount, k);

    cout << "\nРезультат обробки рядків:\n";
    printStringArray(stringArr, strCount);

    // звільнення пам’яті
    freeStudentArray(students, n);
    freeStudentArray(filtered, lowRatingCount);
    delete[] stringArr;
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

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
 
    srand(static_cast<unsigned int>(time(0)));

    runTask1();
    runTask2_1();
    runTask2_2();
    runTask3();

    return 0;
}