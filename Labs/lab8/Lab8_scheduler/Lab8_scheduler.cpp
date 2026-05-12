// програма-планувальник

#include <windows.h>
#include <iostream>
#include <string>

static const int  LAUNCH_COUNT = 3;
static const LONG INTERVAL_SEC = 5;

static bool LaunchAndWait(const std::wstring& exePath, int runIndex) {
    STARTUPINFOW        si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);

    std::wstring cmd = L"\"" + exePath + L"\"";

    if (!CreateProcessW(nullptr, const_cast<wchar_t*>(cmd.data()),
        nullptr, nullptr, FALSE, 0,
        nullptr, nullptr, &si, &pi)) {
        std::wcerr << L"CreateProcess failed: " << GetLastError() << L"\n";
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

int wmain(int argc, wchar_t* argv[]) {
    std::wstring exePath;
    if (argc >= 2) {
        exePath = argv[1];
    }
    else {
        wchar_t selfDir[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, selfDir, MAX_PATH);
        std::wstring self(selfDir);
        auto pos = self.rfind(L'\\');
        exePath = (pos != std::wstring::npos ? self.substr(0, pos + 1) : L"")
            + L"Lab8_main.exe";
    }

    HANDLE timer = CreateWaitableTimerW(nullptr, FALSE, nullptr);
    if (!timer) {
        std::wcerr << L"CreateWaitableTimer failed\n";
        return 1;
    }

    LARGE_INTEGER due{};
    due.QuadPart = 0;
    LONG periodMs = INTERVAL_SEC * 1000;

    if (!SetWaitableTimer(timer, &due, periodMs, nullptr, nullptr, FALSE)) {
        std::wcerr << L"SetWaitableTimer failed\n";
        CloseHandle(timer);
        return 1;
    }


    for (int i = 1; i <= LAUNCH_COUNT; i++) {
        WaitForSingleObject(timer, INFINITE);
        LaunchAndWait(exePath, i);
    }

    CancelWaitableTimer(timer);
    CloseHandle(timer);

    return 0;
}