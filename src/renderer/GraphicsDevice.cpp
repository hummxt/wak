#include "renderer/GraphicsDevice.h"
#include "utils/Logger.h"

namespace ge {

bool GraphicsDevice::Initialize(HWND hwnd, uint32_t width, uint32_t height, bool wallpaperCompatible, bool useComposition) {
    m_hwnd = hwnd;
    m_width = width;
    m_height = height;
    m_wallpaperCompatible = wallpaperCompatible;
    m_useComposition = useComposition;

    if (!CreateDevice()) {
        return false;
    }

    if (!RecreateSwapChain()) {
        return false;
    }

    LogInfo(m_useComposition ? "Graphics device initialized (DirectComposition)" : "Graphics device initialized");
    return true;
}

bool GraphicsDevice::CreateDevice() {
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        levels,
        ARRAYSIZE(levels),
        D3D11_SDK_VERSION,
        &device,
        &featureLevel,
        &context
    );

    if (FAILED(hr)) {
        LogError("Failed to create D3D11 device");
        return false;
    }

    m_device = device;
    m_context = context;
    return true;
}

void GraphicsDevice::ReleaseSwapChain() {
    ReleaseRenderTarget();
    if (m_dcompVisual) {
        m_dcompVisual->SetContent(nullptr);
    }
    m_dcompVisual.Reset();
    m_dcompTarget.Reset();
    m_dcompDevice.Reset();
    m_swapChain1.Reset();
    m_swapChain.Reset();
}

bool GraphicsDevice::CreateHwndSwapChain(HWND hwnd, uint32_t width, uint32_t height) {
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = m_device.As(&dxgiDevice);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IDXGIFactory> factory;
    hr = adapter->GetParent(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount = m_wallpaperCompatible ? 1 : 2;
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = hwnd;
    desc.SampleDesc.Count = 1;
    desc.Windowed = TRUE;
    desc.SwapEffect = m_wallpaperCompatible ? DXGI_SWAP_EFFECT_DISCARD : DXGI_SWAP_EFFECT_FLIP_DISCARD;

    hr = factory->CreateSwapChain(m_device.Get(), &desc, &m_swapChain);
    if (FAILED(hr)) {
        desc.BufferCount = 1;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        hr = factory->CreateSwapChain(m_device.Get(), &desc, &m_swapChain);
        if (FAILED(hr)) {
            LogError("Failed to create HWND swap chain");
            return false;
        }
    }

    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    return true;
}

bool GraphicsDevice::CreateCompositionSwapChain(HWND hwnd, uint32_t width, uint32_t height) {
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = m_device.As(&dxgiDevice);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IDXGIFactory2> factory;
    hr = adapter->GetParent(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        LogError("Failed to get DXGI factory 2");
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.Scaling = DXGI_SCALING_NONE;

    hr = factory->CreateSwapChainForComposition(m_device.Get(), &desc, nullptr, &m_swapChain1);
    if (FAILED(hr)) {
        desc.Scaling = DXGI_SCALING_STRETCH;
        hr = factory->CreateSwapChainForComposition(m_device.Get(), &desc, nullptr, &m_swapChain1);
    }
    if (FAILED(hr)) {
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Scaling = DXGI_SCALING_STRETCH;
        hr = factory->CreateSwapChainForComposition(m_device.Get(), &desc, nullptr, &m_swapChain1);
        if (FAILED(hr)) {
            LogError("Failed to create composition swap chain");
            return false;
        }
    }

    hr = DCompositionCreateDevice(dxgiDevice.Get(), IID_PPV_ARGS(&m_dcompDevice));
    if (FAILED(hr)) {
        LogError("Failed to create DirectComposition device");
        return false;
    }

    hr = m_dcompDevice->CreateTargetForHwnd(hwnd, TRUE, &m_dcompTarget);
    if (FAILED(hr)) {
        LogError("Failed to create DirectComposition target");
        return false;
    }

    hr = m_dcompDevice->CreateVisual(&m_dcompVisual);
    if (FAILED(hr)) {
        return false;
    }

    hr = m_dcompVisual->SetContent(m_swapChain1.Get());
    if (FAILED(hr)) {
        return false;
    }

    hr = m_dcompTarget->SetRoot(m_dcompVisual.Get());
    if (FAILED(hr)) {
        return false;
    }

    hr = m_dcompDevice->Commit();
    if (FAILED(hr)) {
        LogError("DirectComposition Commit failed");
        return false;
    }

    m_swapChain1.As(&m_swapChain);
    return true;
}

bool GraphicsDevice::CreateRenderTarget() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = E_FAIL;
    if (m_swapChain1) {
        hr = m_swapChain1->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    } else if (m_swapChain) {
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    }
    if (FAILED(hr)) {
        return false;
    }

    D3D11_TEXTURE2D_DESC bufferDesc{};
    backBuffer->GetDesc(&bufferDesc);
    if (bufferDesc.Width > 0 && bufferDesc.Height > 0) {
        m_width = bufferDesc.Width;
        m_height = bufferDesc.Height;
    }

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTarget);
    return SUCCEEDED(hr);
}

void GraphicsDevice::ReleaseRenderTarget() {
    if (m_context) {
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
    }
    m_renderTarget.Reset();
}

bool GraphicsDevice::RecreateSwapChain() {
    ReleaseSwapChain();
    bool ok = false;
    if (m_useComposition) {
        ok = CreateCompositionSwapChain(m_hwnd, m_width, m_height);
        if (!ok) {
            LogWarning("Composition swap chain failed, falling back to HWND swap chain");
            m_useComposition = false;
            ok = CreateHwndSwapChain(m_hwnd, m_width, m_height);
        }
    } else {
        ok = CreateHwndSwapChain(m_hwnd, m_width, m_height);
    }

    if (!ok) {
        return false;
    }
    return CreateRenderTarget();
}

void GraphicsDevice::Resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return;
    }

    m_width = width;
    m_height = height;

    if (m_useComposition || !m_swapChain) {
        RecreateSwapChain();
        return;
    }

    ReleaseRenderTarget();
    HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        RecreateSwapChain();
        return;
    }
    CreateRenderTarget();
}

void GraphicsDevice::Present(bool vsync) {
    HRESULT hr = E_FAIL;
    if (m_swapChain1) {
        hr = m_swapChain1->Present(vsync ? 1 : 0, 0);
        if (SUCCEEDED(hr) && m_dcompDevice) {
            m_dcompDevice->Commit();
        }
    } else if (m_swapChain) {
        hr = m_swapChain->Present(vsync ? 1 : 0, 0);
    }

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        LogError("Swapchain Present device lost");
        HandleDeviceLost();
    }
}

void GraphicsDevice::BeginFrame(float r, float g, float b, float a) {
    if (!m_context || !m_renderTarget) {
        return;
    }

    const float color[4] = { r, g, b, a };
    m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), nullptr);
    m_context->ClearRenderTargetView(m_renderTarget.Get(), color);

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &viewport);
}

bool GraphicsDevice::HandleDeviceLost() {
    if (!m_hwnd) {
        return false;
    }

    ReleaseSwapChain();
    m_context.Reset();
    m_device.Reset();
    return Initialize(m_hwnd, m_width, m_height, m_wallpaperCompatible, m_useComposition);
}

ID3D11Device* GraphicsDevice::GetDevice() const {
    return m_device.Get();
}

ID3D11DeviceContext* GraphicsDevice::GetContext() const {
    return m_context.Get();
}

ID3D11RenderTargetView* GraphicsDevice::GetRenderTarget() const {
    return m_renderTarget.Get();
}

uint32_t GraphicsDevice::GetWidth() const {
    return m_width;
}

uint32_t GraphicsDevice::GetHeight() const {
    return m_height;
}

bool GraphicsDevice::UsesComposition() const {
    return m_useComposition;
}

}
