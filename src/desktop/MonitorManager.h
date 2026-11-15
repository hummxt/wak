#pragma once

#include <Windows.h>
#include <vector>
#include <cstdint>
#include <functional>

namespace ge {

struct MonitorInfo {
    HMONITOR handle = nullptr;
    RECT rect{};
    uint32_t width = 0;
    uint32_t height = 0;
    bool primary = false;
};

class MonitorManager {
public:
    void Refresh();
    const std::vector<MonitorInfo>& GetMonitors() const;
    const MonitorInfo* GetPrimary() const;
    bool HasDisplayChanged() const;
    void ClearDisplayChanged();
    void NotifyDisplayChanged();

private:
    static BOOL CALLBACK EnumProc(HMONITOR monitor, HDC, LPRECT, LPARAM data);

    std::vector<MonitorInfo> m_monitors;
    bool m_displayChanged = false;
};

}
