#include "renderer/Renderer.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include <algorithm>

namespace ge {

bool Renderer::Initialize(HWND hwnd, uint32_t width, uint32_t height, const std::wstring& shaderRoot, bool wallpaperCompatible, bool useComposition) {
    m_shaderRoot = shaderRoot;
    if (!m_device.Initialize(hwnd, width, height, wallpaperCompatible, useComposition)) {
        return false;
    }

    m_shaders = new ShaderManager(m_device);
    if (!CreateConstantBuffer()) {
        return false;
    }

    m_params.resolutionX = static_cast<float>(width);
    m_params.resolutionY = static_cast<float>(height);
    return true;
}

bool Renderer::CreateConstantBuffer() {
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(ConstantBufferData);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device.GetDevice()->CreateBuffer(&desc, nullptr, &m_constantBuffer);
    return SUCCEEDED(hr);
}

void Renderer::Resize(uint32_t width, uint32_t height) {
    m_device.Resize(width, height);
    m_params.resolutionX = static_cast<float>(width);
    m_params.resolutionY = static_cast<float>(height);
}

void Renderer::RecreateDeviceSurface() {
    m_device.RecreateSwapChain();
}

void Renderer::Shutdown() {
    m_previewSrv.Reset();
    m_previewRtv.Reset();
    m_previewTexture.Reset();
    m_constantBuffer.Reset();
    if (m_shaders) {
        m_shaders->UnloadAll();
        delete m_shaders;
        m_shaders = nullptr;
    }
}

bool Renderer::SetShader(const std::string& shaderId, const std::wstring& relativePath) {
    if (!m_shaders) {
        return false;
    }

    if (!m_shaders->Has(shaderId)) {
        const std::wstring fullPath = JoinPath(m_shaderRoot, relativePath + L".hlsl");
        if (!m_shaders->LoadShader(shaderId, fullPath)) {
            return false;
        }
    }

    if (m_activeShader != shaderId && !m_activeShader.empty() && m_shaders->Has(m_activeShader)) {
        if (m_activeShader != shaderId) {
            m_shaders->Unload(m_activeShader);
        }
    }

    m_activeShader = shaderId;
    return true;
}

void Renderer::SetPalette(const std::array<float, 4>& a, const std::array<float, 4>& b, const std::array<float, 4>& c, const std::array<float, 4>& d) {
    m_params.colorA = a;
    m_params.colorB = b;
    m_params.colorC = c;
    m_params.colorD = d;
}

void Renderer::UpdateConstants(const FrameParams& params) {
    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = m_device.GetContext()->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr)) {
        return;
    }

    auto* data = static_cast<ConstantBufferData*>(mapped.pData);
    data->time = params.time;
    data->speed = params.speed;
    data->intensity = params.intensity;
    data->brightness = params.brightness;
    data->resolutionX = params.resolutionX;
    data->resolutionY = params.resolutionY;
    data->pad0 = params.angle;
    data->pad1 = 0.0f;
    for (int i = 0; i < 4; ++i) {
        data->colorA[i] = params.colorA[i];
        data->colorB[i] = params.colorB[i];
        data->colorC[i] = params.colorC[i];
        data->colorD[i] = params.colorD[i];
    }

    m_device.GetContext()->Unmap(m_constantBuffer.Get(), 0);
}

void Renderer::DrawFullscreen(const AnimationSettings& animation, float timeSeconds, float width, float height) {
    if (!m_shaders || m_activeShader.empty()) {
        return;
    }

    m_params.time = timeSeconds;
    m_params.speed = animation.speed;
    m_params.intensity = animation.intensity;
    m_params.brightness = animation.brightness;
    m_params.angle = animation.angle;
    m_params.resolutionX = width;
    m_params.resolutionY = height;

    if (!m_shaders->Bind(m_activeShader)) {
        return;
    }

    UpdateConstants(m_params);
    auto* context = m_device.GetContext();
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->Draw(4, 0);
}

void Renderer::Render(const AnimationSettings& animation, float timeSeconds, bool present, bool vsync) {
    m_device.BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);
    DrawFullscreen(animation, timeSeconds, static_cast<float>(m_device.GetWidth()), static_cast<float>(m_device.GetHeight()));
    if (present) {
        m_device.Present(vsync);
    }
}

bool Renderer::EnsurePreviewTarget(uint32_t width, uint32_t height) {
    width = (std::max)(1u, width);
    height = (std::max)(1u, height);
    if (m_previewTexture && m_previewWidth == width && m_previewHeight == height) {
        return true;
    }

    m_previewSrv.Reset();
    m_previewRtv.Reset();
    m_previewTexture.Reset();
    m_previewWidth = width;
    m_previewHeight = height;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device.GetDevice()->CreateTexture2D(&desc, nullptr, &m_previewTexture);
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device.GetDevice()->CreateRenderTargetView(m_previewTexture.Get(), nullptr, &m_previewRtv);
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device.GetDevice()->CreateShaderResourceView(m_previewTexture.Get(), nullptr, &m_previewSrv);
    return SUCCEEDED(hr);
}

void Renderer::RenderPreview(const AnimationSettings& animation, float timeSeconds) {
    if (!m_previewRtv || !m_shaders || m_activeShader.empty()) {
        return;
    }

    auto* context = m_device.GetContext();
    const float clear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->OMSetRenderTargets(1, m_previewRtv.GetAddressOf(), nullptr);
    context->ClearRenderTargetView(m_previewRtv.Get(), clear);

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(m_previewWidth);
    viewport.Height = static_cast<float>(m_previewHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    DrawFullscreen(animation, timeSeconds, static_cast<float>(m_previewWidth), static_cast<float>(m_previewHeight));
}

ID3D11ShaderResourceView* Renderer::GetPreviewSRV() const {
    return m_previewSrv.Get();
}

GraphicsDevice& Renderer::GetDevice() {
    return m_device;
}

ShaderManager& Renderer::GetShaders() {
    return *m_shaders;
}

const std::string& Renderer::GetActiveShader() const {
    return m_activeShader;
}

}
