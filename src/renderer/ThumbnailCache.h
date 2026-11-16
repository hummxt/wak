#pragma once

#include "core/AnimationManager.h"
#include "renderer/Renderer.h"

#include <d3d11.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <wrl/client.h>

namespace ge {

using Microsoft::WRL::ComPtr;

class ThumbnailCache {
public:
    void Clear();
    ID3D11ShaderResourceView* Get(const std::string& presetId) const;
    bool HasMissing(const std::vector<AnimationPreset>& presets) const;
    int BuildMissing(Renderer& renderer, const std::vector<AnimationPreset>& presets, int maxPerFrame);

private:
    struct Entry {
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11RenderTargetView> rtv;
        ComPtr<ID3D11ShaderResourceView> srv;
    };

    bool CreateEntry(Renderer& renderer, Entry& entry);
    bool RenderPreset(Renderer& renderer, const AnimationPreset& preset, Entry& entry);

    std::unordered_map<std::string, Entry> m_entries;
    static constexpr uint32_t kWidth = 320;
    static constexpr uint32_t kHeight = 200;
};

}
