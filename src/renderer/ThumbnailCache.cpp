#include "renderer/ThumbnailCache.h"
#include "utils/FileUtils.h"

namespace ge {

void ThumbnailCache::Clear() {
    m_entries.clear();
    ClearHover();
}

void ThumbnailCache::ClearHover() {
    m_hover = {};
    m_hoverPresetId.clear();
}

ID3D11ShaderResourceView* ThumbnailCache::Get(const std::string& presetId) const {
    const auto it = m_entries.find(presetId);
    if (it == m_entries.end()) {
        return nullptr;
    }
    return it->second.srv.Get();
}

bool ThumbnailCache::CreateEntry(Renderer& renderer, Entry& entry) {
    auto* device = renderer.GetDevice().GetDevice();
    if (!device) {
        return false;
    }

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = kWidth;
    desc.Height = kHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &entry.texture);
    if (FAILED(hr)) {
        return false;
    }
    hr = device->CreateRenderTargetView(entry.texture.Get(), nullptr, &entry.rtv);
    if (FAILED(hr)) {
        return false;
    }
    hr = device->CreateShaderResourceView(entry.texture.Get(), nullptr, &entry.srv);
    return SUCCEEDED(hr);
}

AnimationSettings ThumbnailCache::SettingsFromPreset(const AnimationPreset& preset, bool animate) {
    AnimationSettings settings{};
    settings.speed = animate
        ? (preset.defaultSpeed > 0.05f ? preset.defaultSpeed : 0.45f)
        : 0.0f;
    settings.intensity = preset.defaultIntensity > 0.0f ? preset.defaultIntensity : 0.85f;
    settings.brightness = preset.defaultBrightness;
    settings.angle = preset.defaultAngle;
    settings.zoom = preset.defaultZoom;
    settings.freq = preset.defaultFreq;
    settings.sharpness = preset.defaultSharpness;
    settings.amplitude = preset.defaultAmplitude;
    settings.waveWidthMod = preset.defaultWaveWidthMod;
    settings.offsetX = preset.defaultOffsetX;
    settings.offsetY = preset.defaultOffsetY;
    settings.localWarpIntensity = preset.defaultLocalWarpIntensity;
    settings.localWarpFreqY = preset.defaultLocalWarpFreqY;
    settings.ditherLevels = preset.defaultDitherLevels;
    settings.ditherScale = preset.defaultDitherScale;
    settings.grainIntensity = preset.defaultGrainIntensity;
    settings.grainSpeed = preset.defaultGrainSpeed;
    settings.contrast = preset.defaultContrast;
    settings.vignetteIntensity = preset.defaultVignetteIntensity;
    settings.vignetteRadius = preset.defaultVignetteRadius;
    settings.colorA = preset.colorA;
    settings.colorB = preset.colorB;
    settings.colorC = preset.colorC;
    settings.colorD = preset.colorD;
    return settings;
}

bool ThumbnailCache::RenderPreset(
    Renderer& renderer,
    const AnimationPreset& preset,
    Entry& entry,
    float timeSeconds,
    bool animate
) {
    std::wstring shaderPath = Widen(preset.shader);
    for (auto& ch : shaderPath) {
        if (ch == L'/') {
            ch = L'\\';
        }
    }

    if (!renderer.SetShader(preset.id, shaderPath)) {
        return false;
    }

    AnimationSettings settings = SettingsFromPreset(preset, animate);
    renderer.SetPalette(settings.colorA, settings.colorB, settings.colorC, settings.colorD);
    return renderer.RenderToTarget(entry.rtv.Get(), kWidth, kHeight, settings, timeSeconds);
}

ID3D11ShaderResourceView* ThumbnailCache::RenderHover(
    Renderer& renderer,
    const AnimationPreset& preset,
    float timeSeconds
) {
    if (!m_hover.rtv) {
        if (!CreateEntry(renderer, m_hover)) {
            return nullptr;
        }
    }

    const std::string previousShader = renderer.GetActiveShader();
    if (!RenderPreset(renderer, preset, m_hover, timeSeconds, true)) {
        if (!previousShader.empty()) {
            renderer.ActivateShader(previousShader);
        }
        return nullptr;
    }
    m_hoverPresetId = preset.id;
    if (!previousShader.empty()) {
        renderer.ActivateShader(previousShader);
    }
    return m_hover.srv.Get();
}

bool ThumbnailCache::HasMissing(const std::vector<AnimationPreset>& presets) const {
    for (const auto& preset : presets) {
        if (m_entries.find(preset.id) == m_entries.end()) {
            return true;
        }
    }
    return false;
}

int ThumbnailCache::BuildMissing(Renderer& renderer, const std::vector<AnimationPreset>& presets, int maxPerFrame) {
    int built = 0;
    const std::string previousShader = renderer.GetActiveShader();

    for (const auto& preset : presets) {
        if (built >= maxPerFrame) {
            break;
        }
        if (m_entries.find(preset.id) != m_entries.end()) {
            continue;
        }

        Entry entry;
        if (!CreateEntry(renderer, entry)) {
            continue;
        }
        if (!RenderPreset(renderer, preset, entry, 0.0f, false)) {
            continue;
        }

        m_entries.emplace(preset.id, std::move(entry));
        ++built;
    }

    if (!previousShader.empty()) {
        renderer.ActivateShader(previousShader);
    }

    return built;
}

}
