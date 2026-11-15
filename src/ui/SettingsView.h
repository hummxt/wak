#pragma once

#include "core/Config.h"

#include <functional>

namespace ge {

class SettingsView {
public:
    void Draw(AppConfig& config, const std::function<void()>& onChanged);
};

}
