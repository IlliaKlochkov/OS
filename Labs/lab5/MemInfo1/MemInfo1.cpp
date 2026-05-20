#include <windows.h>
#include <psapi.h>
#include <stdio.h>

#pragma comment(lib, "psapi.lib")

// Допоміжна функція для форматованого виведення заголовку розділу
static void printSection(const char* title)
{
    printf("\n--- %s ---\n", title);
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printf("================================================\n");
    printf("        Програма 1: Системна інформація\n");
    printf("================================================\n");

    // GetSystemInfo повертає апаратні та адресні характеристики системи;
    // виклик відбувається до точки зупинки — щоб адреси вже були визначені
    SYSTEM_INFO si = {};
    GetSystemInfo(&si);

    printSection("GetSystemInfo");
    printf("  Розмір сторінки пам'яті        : %u байт\n", si.dwPageSize);
    printf("  Гранулярність виділення        : %u байт\n", si.dwAllocationGranularity);
    printf("  Мінімальна адреса застосунку   : %p\n", si.lpMinimumApplicationAddress);
    printf("  Максимальна адреса застосунку  : %p\n", si.lpMaximumApplicationAddress);
    printf("  Кількість логічних процесорів  : %u\n", si.dwNumberOfProcessors);

    // GlobalMemoryStatusEx дає миттєвий знімок стану всіх типів пам'яті
    MEMORYSTATUSEX ms = {};
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatusEx(&ms);

    printSection("GlobalMemoryStatusEx");
    printf("  Завантаженість пам'яті         : %u %%\n", ms.dwMemoryLoad);
    printf("  Всього фізичної пам'яті        : %llu МБ\n", ms.ullTotalPhys / (1024ULL * 1024));
    printf("  Вільно фізичної пам'яті        : %llu МБ\n", ms.ullAvailPhys / (1024ULL * 1024));
    printf("  Всього файлу підкачки          : %llu МБ\n", ms.ullTotalPageFile / (1024ULL * 1024));
    printf("  Вільно файлу підкачки          : %llu МБ\n", ms.ullAvailPageFile / (1024ULL * 1024));
    printf("  Всього віртуальної пам'яті     : %llu МБ\n", ms.ullTotalVirtual / (1024ULL * 1024));
    printf("  Вільно віртуальної пам'яті     : %llu МБ\n", ms.ullAvailVirtual / (1024ULL * 1024));

    // Базова адреса модуля = початок завантаженого образу PE в адресному просторі процесу;
    // GetModuleInformation дає точний розмір через SizeOfImage
    HMODULE hMod = GetModuleHandle(NULL);
    MODULEINFO modInfo = {};
    GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(modInfo));

    printSection("Адреси програми 1");
    printf("  Базова адреса (початок образу) : %p\n", modInfo.lpBaseOfDll);
    printf("  Розмір образу (SizeOfImage)    : %u байт\n", modInfo.SizeOfImage);
    printf("  Кінець образу (приблизно)      : %p\n", (BYTE*)modInfo.lpBaseOfDll + modInfo.SizeOfImage);
    printf("  Адреса функції main()          : %p\n", (void*)main);
    printf("  PID поточного процесу          : %u\n", GetCurrentProcessId());

	printf("\n>>> ТОЧКА ЗУПИНКИ <<<\n");
    getchar();
    return 0;
}