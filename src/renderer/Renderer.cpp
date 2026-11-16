#include "renderer/Renderer.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include <algorithm>

namespace ge {

bool Renderer::Initialize(HWND hwnd, uint32_t width, uint32_t height, const std::wstring& shaderRoot, bool wallpaperCompatible, bool useComposition) {
    m_shaderRoot = shaderRoot;
    m_highQuality = wallpaperCompatible;
    if (!m_device.Initialize(hwnd, width, height, wallpaperCompatible, useComposition)) {
        return false;
    }

    m_shaders = new ShaderManager(m_device);
    if (!CreateConstantBuffer()) {
        return false;
    }
    if (!CreateSampler()) {
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

bool Renderer::CreateSampler() {
    D3D11_SAMPLER_DESC desc{};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT hr = m_device.GetDevice()->CreateSamplerState(&desc, &m_linearSampler);
    return SUCCEEDED(hr);
}

bool Renderer::EnsureHqTarget(uint32_t width, uint32_t height) {
    width = (std::max)(1u, width);
    height = (std::max)(1u, height);
    if (m_hqTexture && m_hqWidth == width && m_hqHeight == height) {
        return true;
    }

    m_hqSrv.Reset();
    m_hqRtv.Reset();
    m_hqTexture.Reset();
    m_hqWidth = width;
    m_hqHeight = height;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device.GetDevice()->CreateTexture2D(&desc, nullptr, &m_hqTexture);
    if (FAILED(hr)) {
        return false;
    }
    hr = m_device.GetDevice()->CreateRenderTargetView(m_hqTexture.Get(), nullptr, &m_hqRtv);
    if (FAILED(hr)) {
        return false;
    }
    hr = m_device.GetDevice()->CreateShaderResourceView(m_hqTexture.Get(), nullptr, &m_hqSrv);
    return SUCCEEDED(hr);
}

bool Renderer::EnsureBlitShader() {
    if (!m_shaders) {
        return false;
    }
    if (m_shaders->Has("blit-dither")) {
        return true;
    }
    const std::wstring path = JoinPath(m_shaderRoot, L"common\\BlitDither.hlsl");
    return m_shaders->LoadShader("blit-dither", path);
}

void Renderer::Resize(uint32_t width, uint32_t height) {
    m_device.Resize(width, height);
    m_params.resolutionX = static_cast<float>(m_device.GetWidth());
    m_params.resolutionY = static_cast<float>(m_device.GetHeight());
    m_hqSrv.Reset();
    m_hqRtv.Reset();
    m_hqTexture.Reset();
    m_hqWidth = 0;
    m_hqHeight = 0;
}

void Renderer::RecreateDeviceSurface() {
    m_device.RecreateSwapChain();
    m_params.resolutionX = static_cast<float>(m_device.GetWidth());
    m_params.resolutionY = static_cast<float>(m_device.GetHeight());
}

void Renderer::SetHighQuality(bool enabled) {
    m_highQuality = enabled;
}

void Renderer::Shutdown() {
    m_hqSrv.Reset();
    m_hqRtv.Reset();
    m_hqTexture.Reset();
    m_previewSrv.Reset();
    m_previewRtv.Reset();
    m_previewTexture.Reset();
    m_linearSampler.Reset();
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
        if (relativePath.empty()) {
            return false;
        }
        const std::wstring fullPath = JoinPath(m_shaderRoot, relativePath + L".hlsl");
        if (!m_shaders->LoadShader(shaderId, fullPath)) {
            return false;
        }
    }

    m_activeShader = shaderId;
    return true;
}

bool Renderer::ActivateShader(const std::string& shaderId) {
    if (!m_shaders || !m_shaders->Has(shaderId)) {
        return false;
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
    m_params.colorA = animation.colorA;
    m_params.colorB = animation.colorB;
    m_params.colorC = animation.colorC;
    m_params.colorD = animation.colorD;

    if (!m_shaders->Bind(m_activeShader)) {
        return;
    }

    UpdateConstants(m_params);
    auto* context = m_device.GetContext();
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->Draw(4, 0);
}

void Renderer::BlitHqToBackbuffer(float width, float height) {
    if (!m_hqSrv || !EnsureBlitShader() || !m_linearSampler) {
        return;
    }

    const std::string previous = m_activeShader;
    if (!m_shaders->Bind("blit-dither")) {
        return;
    }

    m_params.resolutionX = width;
    m_params.resolutionY = height;
    UpdateConstants(m_params);

    auto* context = m_device.GetContext();
    ID3D11ShaderResourceView* srv = m_hqSrv.Get();
    context->PSSetShaderResources(0, 1, &srv);
    context->PSSetSamplers(0, 1, m_linearSampler.GetAddressOf());
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->Draw(4, 0);

    ID3D11ShaderResourceView* nullSrv = nullptr;
    context->PSSetShaderResources(0, 1, &nullSrv);

    if (!previous.empty()) {
        ActivateShader(previous);
    }
}

void Renderer::Render(const AnimationSettings& animation, float timeSeconds, bool present, bool vsync) {
    const uint32_t outW = m_device.GetWidth();
    const uint32_t outH = m_device.GetHeight();

    if (m_highQuality && outW > 0 && outH > 0) {
        const uint32_t hqW = outW * 2;
        const uint32_t hqH = outH * 2;
        if (EnsureHqTarget(hqW, hqH) && m_hqRtv) {
            RenderToTarget(m_hqRtv.Get(), hqW, hqH, animation, timeSeconds);
            m_device.BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);
            BlitHqToBackbuffer(static_cast<float>(outW), static_cast<float>(outH));
            if (present) {
                m_device.Present(vsync);
            }
            return;
        }
    }

    m_device.BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);
    DrawFullscreen(animation, timeSeconds, static_cast<float>(outW), static_cast<float>(outH));
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
    if (!m_previewRtv) {
        return;
    }
    RenderToTarget(m_previewRtv.Get(), m_previewWidth, m_previewHeight, animation, timeSeconds);
}

bool Renderer::RenderToTarget(ID3D11RenderTargetView* rtv, uint32_t width, uint32_t height, const AnimationSettings& animation, float timeSeconds) {
    if (!rtv || !m_shaders || m_activeShader.empty() || width == 0 || height == 0) {
        return false;
    }

    auto* context = m_device.GetContext();
    const float clear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->OMSetRenderTargets(1, &rtv, nullptr);
    context->ClearRenderTargetView(rtv, clear);

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    DrawFullscreen(animation, timeSeconds, static_cast<float>(width), static_cast<float>(height));
    return true;
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
