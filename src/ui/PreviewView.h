#pragma once

#include "core/AnimationManager.h"
#include "core/Config.h"

#include <d3d11.h>
#include <functional>
#include <string>

namespace ge {

class PreviewView {
public:
    void Draw(
        const AnimationPreset* preset,
        AnimationSettings& animation,
        bool isFavorite,
        ID3D11ShaderResourceView* previewSrv,
        const std::string& desktopStatus,
        const std::function<void()>& onApply,
        const std::function<void()>& onToggleFavorite,
        const std::function<void()>& onSettingsChanged
    );
};

}
