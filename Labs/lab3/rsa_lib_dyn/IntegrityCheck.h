#pragma once
#include <windows.h>

// Перевіряє контрольну суму DLL-файлу під час її завантаження
bool VerifyDllChecksum(HMODULE hModule);
