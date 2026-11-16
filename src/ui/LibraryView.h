#pragma once

#include "core/AnimationManager.h"

#include <d3d11.h>
#include <functional>
#include <string>

namespace ge {

struct LibraryViewState {
    std::string selectedCategory = "All";
    std::string searchQuery;
    std::string selectedPresetId;
    int sidebarIndex = 0;
};

class LibraryView {
public:
    void Draw(
        AnimationManager& animations,
        LibraryViewState& state,
        const std::function<ID3D11ShaderResourceView*(const std::string&)>& getThumbnail,
        const std::function<void(const AnimationPreset&)>& onSelect
    );

private:
    float m_scrollCurrent = 0.0f;
    float m_scrollTarget = 0.0f;
    bool m_scrollInitialized = false;
};

}
