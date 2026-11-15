#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <wrl/client.h>
#include <cstdint>

namespace ge {

using Microsoft::WRL::ComPtr;

class GraphicsDevice {
public:
    bool Initialize(HWND hwnd, uint32_t width, uint32_t height, bool wallpaperCompatible = false, bool useComposition = false);
    void Resize(uint32_t width, uint32_t height);
    void Present(bool vsync);
    void BeginFrame(float r, float g, float b, float a);
    bool RecreateSwapChain();
    bool HandleDeviceLost();

    ID3D11Device* GetDevice() const;
    ID3D11DeviceContext* GetContext() const;
    ID3D11RenderTargetView* GetRenderTarget() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    bool UsesComposition() const;

private:
    bool CreateDevice();
    bool CreateHwndSwapChain(HWND hwnd, uint32_t width, uint32_t height);
    bool CreateCompositionSwapChain(HWND hwnd, uint32_t width, uint32_t height);
    bool CreateRenderTarget();
    void ReleaseRenderTarget();
    void ReleaseSwapChain();

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<IDXGISwapChain1> m_swapChain1;
    ComPtr<IDCompositionDevice> m_dcompDevice;
    ComPtr<IDCompositionTarget> m_dcompTarget;
    ComPtr<IDCompositionVisual> m_dcompVisual;
    ComPtr<ID3D11RenderTargetView> m_renderTarget;
    HWND m_hwnd = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_wallpaperCompatible = false;
    bool m_useComposition = false;
};

}
