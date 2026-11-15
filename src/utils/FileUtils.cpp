#include "utils/FileUtils.h"

#include <Windows.h>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace ge {
namespace fs = std::filesystem;

std::string ReadTextFile(const std::wstring& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return {};
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool WriteTextFile(const std::wstring& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return static_cast<bool>(file);
}

bool FileExists(const std::wstring& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

bool DirectoryExists(const std::wstring& path) {
    return fs::exists(path) && fs::is_directory(path);
}

std::wstring GetExecutableDirectory() {
    wchar_t buffer[MAX_PATH]{};
    const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        return L".";
    }
    fs::path p(buffer);
    return p.parent_path().wstring();
}

std::wstring JoinPath(const std::wstring& a, const std::wstring& b) {
    return (fs::path(a) / b).wstring();
}

std::vector<std::wstring> ListFiles(const std::wstring& directory, const std::wstring& extension) {
    std::vector<std::wstring> result;
    if (!DirectoryExists(directory)) {
        return result;
    }
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (!extension.empty() && entry.path().extension().wstring() != extension) {
            continue;
        }
        result.push_back(entry.path().wstring());
    }
    return result;
}

std::string Narrow(const std::wstring& wide) {
    if (wide.empty()) {
        return {};
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    std::string out(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), out.data(), size, nullptr, nullptr);
    return out;
}

std::wstring Widen(const std::string& narrow) {
    if (narrow.empty()) {
        return {};
    }
    const int size = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), static_cast<int>(narrow.size()), nullptr, 0);
    std::wstring out(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), static_cast<int>(narrow.size()), out.data(), size);
    return out;
}

}
