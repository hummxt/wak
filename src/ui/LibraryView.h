#pragma once

#include "core/AnimationManager.h"

#include <string>
#include <functional>

namespace ge {

struct LibraryViewState {
    std::string selectedCategory = "All";
    std::string searchQuery;
    std::string selectedPresetId;
    int sidebarIndex = 0;
};

class LibraryView {
public:
    void Draw(AnimationManager& animations, LibraryViewState& state, const std::function<void(const AnimationPreset&)>& onSelect);
};

}
