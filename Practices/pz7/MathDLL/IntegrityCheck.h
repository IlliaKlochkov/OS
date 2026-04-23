#pragma once
#include <windows.h>

// Перевіряє контрольну суму DLL-файлу під час його завантаження
bool VerifyDllChecksum(HMODULE hModule);