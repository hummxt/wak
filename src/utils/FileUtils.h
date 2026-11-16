#pragma once

#include <string>
#include <vector>
#include <optional>

namespace ge {

std::string ReadTextFile(const std::wstring& path);
bool WriteTextFile(const std::wstring& path, const std::string& content);
bool FileExists(const std::wstring& path);
bool DirectoryExists(const std::wstring& path);
std::wstring GetExecutableDirectory();
std::wstring GetAppDataDirectory();
bool EnsureDirectoryExists(const std::wstring& path);
std::wstring JoinPath(const std::wstring& a, const std::wstring& b);
std::vector<std::wstring> ListFiles(const std::wstring& directory, const std::wstring& extension);
std::string Narrow(const std::wstring& wide);
std::wstring Widen(const std::string& narrow);

}
