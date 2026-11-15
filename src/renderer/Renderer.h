#pragma once

#include "renderer/GraphicsDevice.h"
#include "renderer/ShaderManager.h"
#include "core/Config.h"

#include <array>
#include <string>

namespace ge {

struct FrameParams {
    float time = 0.0f;
    float speed = 0.5f;
    float intensity = 0.7f;
    float brightness = 1.0f;
    float resolutionX = 1.0f;
    float resolutionY = 1.0f;
    std::array<float, 4> colorA{ 0.45f, 0.20f, 0.85f, 1.0f };
    std::array<float, 4> colorB{ 0.15f, 0.35f, 0.95f, 1.0f };
    std::array<float, 4> colorC{ 0.90f, 0.35f, 0.55f, 1.0f };
    std::array<float, 4> colorD{ 0.10f, 0.75f, 0.80f, 1.0f };
};

class Renderer {
public:
    bool Initialize(HWND hwnd, uint32_t width, uint32_t height, const std::wstring& shaderRoot, bool wallpaperCompatible = false, bool useComposition = false);
    void Resize(uint32_t width, uint32_t height);
    void Shutdown();

    bool SetShader(const std::string& shaderId, const std::wstring& relativePath);
    void SetPalette(const std::array<float, 4>& a, const std::array<float, 4>& b, const std::array<float, 4>& c, const std::array<float, 4>& d);
    void Render(const AnimationSettings& animation, float timeSeconds, bool present, bool vsync);
    bool EnsurePreviewTarget(uint32_t width, uint32_t height);
    void RenderPreview(const AnimationSettings& animation, float timeSeconds);
    ID3D11ShaderResourceView* GetPreviewSRV() const;
    void RecreateDeviceSurface();

    GraphicsDevice& GetDevice();
    ShaderManager& GetShaders();
    const std::string& GetActiveShader() const;

private:
    bool CreateConstantBuffer();
    void UpdateConstants(const FrameParams& params);
    void DrawFullscreen(const AnimationSettings& animation, float timeSeconds, float width, float height);

    struct alignas(16) ConstantBufferData {
        float time;
        float speed;
        float intensity;
        float brightness;
        float resolutionX;
        float resolutionY;
        float pad0;
        float pad1;
        float colorA[4];
        float colorB[4];
        float colorC[4];
        float colorD[4];
    };

    GraphicsDevice m_device;
    ShaderManager* m_shaders = nullptr;
    ComPtr<ID3D11Buffer> m_constantBuffer;
    ComPtr<ID3D11Texture2D> m_previewTexture;
    ComPtr<ID3D11RenderTargetView> m_previewRtv;
    ComPtr<ID3D11ShaderResourceView> m_previewSrv;
    uint32_t m_previewWidth = 0;
    uint32_t m_previewHeight = 0;
    std::wstring m_shaderRoot;
    std::string m_activeShader;
    FrameParams m_params;
};

}
