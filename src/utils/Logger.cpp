#include "utils/Logger.h"

#include <Windows.h>
#include <iostream>

namespace ge {

void Log(LogLevel level, const std::string& message) {
    const char* prefix = "INFO";
    if (level == LogLevel::Warning) {
        prefix = "WARN";
    } else if (level == LogLevel::Error) {
        prefix = "ERROR";
    }

    const std::string line = std::string("[") + prefix + "] " + message + "\n";
    OutputDebugStringA(line.c_str());

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console && console != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteConsoleA(console, line.c_str(), static_cast<DWORD>(line.size()), &written, nullptr);
    }
}

void LogInfo(const std::string& message) {
    Log(LogLevel::Info, message);
}

void LogWarning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void LogError(const std::string& message) {
    Log(LogLevel::Error, message);
}

}
