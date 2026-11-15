#include "desktop/MonitorManager.h"

namespace ge {

BOOL CALLBACK MonitorManager::EnumProc(HMONITOR monitor, HDC, LPRECT, LPARAM data) {
    auto* self = reinterpret_cast<MonitorManager*>(data);
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) {
        return TRUE;
    }

    MonitorInfo entry;
    entry.handle = monitor;
    entry.rect = info.rcMonitor;
    entry.width = static_cast<uint32_t>(info.rcMonitor.right - info.rcMonitor.left);
    entry.height = static_cast<uint32_t>(info.rcMonitor.bottom - info.rcMonitor.top);
    entry.primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
    self->m_monitors.push_back(entry);
    return TRUE;
}

void MonitorManager::Refresh() {
    m_monitors.clear();
    EnumDisplayMonitors(nullptr, nullptr, EnumProc, reinterpret_cast<LPARAM>(this));
}

const std::vector<MonitorInfo>& MonitorManager::GetMonitors() const {
    return m_monitors;
}

const MonitorInfo* MonitorManager::GetPrimary() const {
    for (const auto& monitor : m_monitors) {
        if (monitor.primary) {
            return &monitor;
        }
    }
    if (!m_monitors.empty()) {
        return &m_monitors.front();
    }
    return nullptr;
}

bool MonitorManager::HasDisplayChanged() const {
    return m_displayChanged;
}

void MonitorManager::ClearDisplayChanged() {
    m_displayChanged = false;
}

void MonitorManager::NotifyDisplayChanged() {
    m_displayChanged = true;
    Refresh();
}

}
