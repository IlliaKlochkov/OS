#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h> // для роботи з файлами у стилі C
#include <fstream> // для роботи з файлами у стилі C++

using namespace std;

struct Buyer {
    char fullname[100];
    char address[100];
    char phone[20];
    char card_number[20];

    // статичний метод щоб зручно створювати елементи
    static Buyer create() {
        Buyer b;

        cout << "1. Введіть (Повне ім'я) Ім'я Прізвище Побатькові: ";
        cin.getline(b.fullname, 100); // отрмуємо всю строчку, щоб отримати рядок з пробілами

        cout << "2. Введіть Адресу: ";;
        cin.getline(b.address, 100);

        cout << "3. Введіть Номер телефону: ";
        cin >> b.phone;

        cout << "4. Введіть Номер картки: ";
        cin >> b.card_number;

        cin.ignore(10000, '\n');

        return b;
    }

    // статичний метод для створення масиву з покупцями
    static Buyer* list(int size_of_arr) {
        Buyer* arr = new Buyer[size_of_arr];

        // створюємо потрібну кількість елементів
        for (int i = 0; i < size_of_arr; i++) {
            printf("\n+-- Створення елемента №%d --+\n", i + 1);

            arr[i] = Buyer::create();
            cout << "\n";
        }

        return arr;
    }
};

// ==========================================
// Стиль C
// ==========================================

void createFile_C(const char* filename, Buyer* buyers, int count) {
    FILE* file = fopen(filename, "wb");
    
    if (!file) {
        perror("Помилка при відкритті файлу");
        return;
    }

    fwrite(buyers, sizeof(Buyer), count, file);
    fclose(file);
}

void addToFile_C(const char* filename, const char* filename_temp, Buyer* newBuyers, int n, int k) {
    FILE* file = fopen(filename, "rb");

    if (!file) {
        perror("Помилка при відкритті файлу");
        return;
    }

    // створюємо тимчасовий файл
    FILE* temp = fopen(filename_temp, "wb");
    
    Buyer b;
    int index = 0;
    bool is_added = false;

    // прходимось по всім записаним елементам поким не натрпимо на елемент під номером K 
    // коли знаходимо його то додаємо нові елементи
    // якщо ми пройшлись повністю і не на трапили на K, то просто додаємо в кінці файлу нові елементи
    while (fread(&b, sizeof(Buyer), 1, file)) {
        if (index == k && !is_added) {
            fwrite(newBuyers, sizeof(Buyer), n, temp);
            is_added = true;
        }
        
        fwrite(&b, sizeof(Buyer), 1, temp);
        
        index++;
    }

    if (!is_added) {
        fwrite(newBuyers, sizeof(Buyer), n, temp); 
    }

    
    fclose(file);
    fclose(temp);

    // заміняємо зміст файлів
    remove(filename);
    rename(filename_temp, filename);
}

void deleteFromFile_C(const char* filename, const char* filename_temp, int k) {
    FILE* file = fopen(filename, "rb");

    if (!file) {
        perror("Помилка при відкритті файлу");
        return;
    }

    // створюємо тимчасовий файл
    FILE* temp = fopen(filename_temp, "wb");

    Buyer b;
    int index = 0;

    // проходимось по всім елементам, але не записуємо перші k елементів
    while (fread(&b, sizeof(Buyer), 1, file)) {
        if (index >= k) {
            fwrite(&b, sizeof(Buyer), 1, temp);
        }
        index++;
    }
    
    fclose(file);
    fclose(temp);
    
    // заміняємо зміст файлів
    remove(filename);
    rename(filename_temp, filename);
    
    return;
}

void printFile_C(const char* filename) {
    FILE* file = fopen(filename, "rb");

    if (!file) {
        perror("Помилка при відкритті файлу");
        return;
    }

    Buyer b;

    printf(">> Вміст файлу у стилі C: \n");
    while (fread(&b, sizeof(Buyer), 1, file) == 1) {
        printf("%s, Адреса: %s, Тел: %s, Картка: %s\n",
            b.fullname, b.address, b.phone, b.card_number);
    }

    fclose(file);
}

// ==========================================
// Стиль C++
// ==========================================

void createFile_CPP(const char* filename, Buyer* buyers, int count) {
    ofstream file(filename, ios::binary);
    
    if (!file) {
        cerr << "Помилка створення файлу \n";
        return;
    }

    file.write((char*)buyers, sizeof(Buyer) * count);
    file.close();
}

void addToFile_CPP(const char* filename, const char* filename_temp, Buyer* newBuyers, int n, int k) {
    ifstream file(filename, ios::binary);

    if (!file) {
        perror("Помилка при відкритті файлу");
        return;
    }

    ofstream temp(filename_temp, ios::binary);

    Buyer b;
    int index = 0;
    bool is_added = false;

    while (file.read((char*)&b, sizeof(Buyer))) {
        if (index == k && !is_added) {
            temp.write((char*)newBuyers, sizeof(Buyer) * n);

            is_added = true;
        }

        temp.write((char*)&b, sizeof(Buyer));

        index++;
    }

    if (!is_added) {
        temp.write((char*)newBuyers, sizeof(Buyer) * n);
    }

    file.close();
    temp.close();

    remove(filename);
    rename(filename_temp, filename);

    return;
}

void deleteFromFile_CPP(const char* filename, const char* filename_temp, int k) {
    ifstream file(filename, ios::binary);

    if (!file) {
        cerr << "Помилка відкриття файлу \n";
        return;
    }

    ofstream temp(filename_temp, ios::binary);

    Buyer b;
    int index = 0;

    while (file.read((char*)&b, sizeof(Buyer))) {
        if (index >= k) {
            temp.write((char*)&b, sizeof(Buyer));
        }
        index++;
    }

    file.close();
    temp.close();

    remove(filename);
    rename(filename_temp, filename);
    return;
}

void printFile_CPP(const char* filename) {
    ifstream file(filename, ios::binary);

    if (!file) {
        cerr << "Помилка відкриття файлу \n";
        return;
    }

    Buyer buyer;

    cout << ">> Вміст файлу у стилі C++: \n";

    while (file.read((char*)&buyer, sizeof(Buyer))) {
        cout << buyer.fullname << " " << ", Адреса: " << buyer.address << ", Тел: " 
             << buyer.phone << ", Картка: " << buyer.card_number << "\n";
    };

    file.close();
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
    int n, n_add;           // n - для buyers, n_add - для new_buyers
    int k_add, k_delete;    // k для додавання або видалення
    Buyer* buyers;          // масив для перших елементів
    Buyer* new_buyers;      // масив для додавання нових елементів

    // використовуємо константи для назв файлів
    const char* filename_c = "buyers_c.dat";
    const char* filename_cpp = "buyers_cpp.dat";
    const char* filename_temp = "temp.dat";

    cout << ">> Введіть початковий розмір масиву Покупців: ";
    cin >> n;
    cin.ignore(10000, '\n');

    // використовуємо статичний метод для ініціалізації першого масиву елементів
    buyers = Buyer::list(n);

    cout << "\n+-----------------------------------+";
    cout << "\n|     Обробка файлу у стилі C       |";
    cout << "\n+-----------------------------------+\n\n";

    createFile_C(filename_c, buyers, n);
    printFile_C(filename_c);

    // + Видалення + 
    cout << "\n>> Введіть кількість елементів для видалення з початку файлу: ";
    cin >> k_delete; 
    deleteFromFile_C(filename_c, filename_temp, k_delete);

    // + Додавання + 
    cout << ">> Введіть кількість Покупців для додавання: ";
    cin >> n_add;

    cout << ">> Введіть номер куди вставити нові елементи: ";
    cin >> k_add;
    cin.ignore(10000, '\n');
    
    
    new_buyers = Buyer::list(n_add);

    addToFile_C(filename_c, filename_temp, new_buyers, n_add, k_add);
    printFile_C(filename_c);

    // ------------------------ 
    // формат C++
    // ------------------------ 
    n_add = 0;              // скидаємо значенн для коректної роботи 
    k_add = 0;
    k_delete = 0;

    cout << "\n+-----------------------------------+";
    cout << "\n|    Обробка файлу у стилі C++      |";
    cout << "\n+-----------------------------------+\n";

    createFile_CPP(filename_cpp, buyers, n);
    printFile_CPP(filename_cpp);

    // + Видалення + 
    cout << "\n>> Введіть кількість елементів для видалення з початку файлу: ";
    cin >> k_delete;
    deleteFromFile_CPP(filename_cpp, filename_temp, k_delete);

    // + Додавання + 
    cout << ">> Введіть кількість Покупців для додавання: ";
    cin >> n_add;

    cout << ">> Введіть номер куди вставити нові елементи: ";
    cin >> k_add;
    cin.ignore(10000, '\n');

    delete[] new_buyers;
    new_buyers = Buyer::list(n_add);

    addToFile_CPP(filename_cpp, filename_temp, new_buyers, n_add, k_add);
    printFile_CPP(filename_cpp);

    delete[] buyers;
    delete[] new_buyers;
    
    remove(filename_c);
    remove(filename_cpp);
    return 0;
}