#include "utils/TextureLoader.h"
#include "utils/Logger.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <wincodec.h>

#pragma comment(lib, "windowscodecs.lib")

namespace ge {
namespace {

float RoundedRectCoverage(float x, float y, float size, float radius) {
    if (size <= 0.0f) {
        return 0.0f;
    }
    const float r = (std::max)(0.0f, (std::min)(radius, size * 0.5f));
    const float px = (std::min)(x, size - 1.0f - x);
    const float py = (std::min)(y, size - 1.0f - y);
    if (px >= r || py >= r) {
        return 1.0f;
    }
    const float dx = r - px;
    const float dy = r - py;
    const float dist = std::sqrt(dx * dx + dy * dy);
    const float edge = dist - r;
    if (edge <= -0.75f) {
        return 1.0f;
    }
    if (edge >= 0.75f) {
        return 0.0f;
    }
    return 1.0f - ((edge + 0.75f) / 1.5f);
}

void SampleBilinearBGRA(
    const BYTE* src,
    int srcW,
    int srcH,
    float u,
    float v,
    BYTE out[4]
) {
    if (!src || srcW <= 0 || srcH <= 0) {
        out[0] = out[1] = out[2] = out[3] = 0;
        return;
    }
    u = (std::max)(0.0f, (std::min)(u, 1.0f));
    v = (std::max)(0.0f, (std::min)(v, 1.0f));
    const float fx = u * static_cast<float>(srcW - 1);
    const float fy = v * static_cast<float>(srcH - 1);
    const int x0 = static_cast<int>(fx);
    const int y0 = static_cast<int>(fy);
    const int x1 = (std::min)(x0 + 1, srcW - 1);
    const int y1 = (std::min)(y0 + 1, srcH - 1);
    const float tx = fx - static_cast<float>(x0);
    const float ty = fy - static_cast<float>(y0);

    const auto at = [&](int x, int y) -> const BYTE* {
        return src + (static_cast<size_t>(y) * static_cast<size_t>(srcW) + static_cast<size_t>(x)) * 4;
    };
    const BYTE* c00 = at(x0, y0);
    const BYTE* c10 = at(x1, y0);
    const BYTE* c01 = at(x0, y1);
    const BYTE* c11 = at(x1, y1);
    for (int i = 0; i < 4; ++i) {
        const float a = static_cast<float>(c00[i]) + (static_cast<float>(c10[i]) - static_cast<float>(c00[i])) * tx;
        const float b = static_cast<float>(c01[i]) + (static_cast<float>(c11[i]) - static_cast<float>(c01[i])) * tx;
        out[i] = static_cast<BYTE>((std::max)(0.0f, (std::min)(255.0f, a + (b - a) * ty + 0.5f)));
    }
}

}



bool LoadTextureFromFile(
    ID3D11Device* device,
    const std::wstring& path,
    ComPtr<ID3D11ShaderResourceView>& outSrv,
    int* outWidth,
    int* outHeight
) {
    outSrv.Reset();
    if (!device || path.empty()) {
        return false;
    }

    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );
    if (FAILED(hr)) {
        LogError("Failed to create WIC factory");
        return false;
    }

    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromFilename(
        path.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    if (FAILED(hr)) {
        LogError("Failed to decode image");
        return false;
    }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        return false;
    }

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) {
        return false;
    }

    UINT width = 0;
    UINT height = 0;
    hr = converter->GetSize(&width, &height);
    if (FAILED(hr) || width == 0 || height == 0) {
        return false;
    }

    const UINT stride = width * 4;
    const UINT imageSize = stride * height;
    std::vector<BYTE> pixels(imageSize);
    hr = converter->CopyPixels(nullptr, stride, imageSize, pixels.data());
    if (FAILED(hr)) {
        return false;
    }

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = pixels.data();
    init.SysMemPitch = stride;

    ComPtr<ID3D11Texture2D> texture;
    hr = device->CreateTexture2D(&desc, &init, &texture);
    if (FAILED(hr)) {
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &outSrv);
    if (FAILED(hr)) {
        return false;
    }

    if (outWidth) {
        *outWidth = static_cast<int>(width);
    }
    if (outHeight) {
        *outHeight = static_cast<int>(height);
    }
    return true;
}

HICON LoadIconFromFile(
    const std::wstring& path,
    int width,
    int height,
    float paddingRatio,
    float cornerRadiusRatio
) {
    if (path.empty() || width <= 0 || height <= 0) {
        return nullptr;
    }

    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );
    if (FAILED(hr)) {
        return nullptr;
    }

    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromFilename(
        path.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    if (FAILED(hr)) {
        return nullptr;
    }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        return nullptr;
    }

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        return nullptr;
    }

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) {
        return nullptr;
    }

    UINT srcW = 0;
    UINT srcH = 0;
    hr = converter->GetSize(&srcW, &srcH);
    if (FAILED(hr) || srcW == 0 || srcH == 0) {
        return nullptr;
    }

    const UINT srcStride = srcW * 4;
    const UINT srcSize = srcStride * srcH;
    std::vector<BYTE> srcPixels(srcSize);
    hr = converter->CopyPixels(nullptr, srcStride, srcSize, srcPixels.data());
    if (FAILED(hr)) {
        return nullptr;
    }

    const float pad = (std::max)(0.0f, (std::min)(paddingRatio, 0.4f));
    const float radiusRatio = (std::max)(0.0f, (std::min)(cornerRadiusRatio, 0.5f));
    const float sizeF = static_cast<float>((std::min)(width, height));
    const float content = sizeF * (1.0f - pad * 2.0f);
    const float offsetX = (static_cast<float>(width) - content) * 0.5f;
    const float offsetY = (static_cast<float>(height) - content) * 0.5f;
    const float radius = content * radiusRatio;

    const UINT stride = static_cast<UINT>(width) * 4;
    const UINT imageSize = stride * static_cast<UINT>(height);
    std::vector<BYTE> pixels(imageSize, 0);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            BYTE* dst = pixels.data() + (static_cast<size_t>(y) * width + static_cast<size_t>(x)) * 4;
            const float lx = static_cast<float>(x) + 0.5f - offsetX;
            const float ly = static_cast<float>(y) + 0.5f - offsetY;
            if (lx < 0.0f || ly < 0.0f || lx >= content || ly >= content) {
                continue;
            }

            const float coverage = RoundedRectCoverage(lx, ly, content, radius);
            if (coverage <= 0.001f) {
                continue;
            }

            BYTE sample[4]{};
            SampleBilinearBGRA(
                srcPixels.data(),
                static_cast<int>(srcW),
                static_cast<int>(srcH),
                lx / content,
                ly / content,
                sample
            );

            const float alpha = static_cast<float>(sample[3]) * coverage;
            dst[0] = sample[0];
            dst[1] = sample[1];
            dst[2] = sample[2];
            dst[3] = static_cast<BYTE>((std::max)(0.0f, (std::min)(255.0f, alpha + 0.5f)));
        }
    }

    ICONINFO info{};
    info.fIcon = TRUE;
    info.hbmColor = CreateBitmap(width, height, 1, 32, pixels.data());
    info.hbmMask = CreateBitmap(width, height, 1, 1, nullptr);
    if (!info.hbmColor || !info.hbmMask) {
        if (info.hbmColor) {
            DeleteObject(info.hbmColor);
        }
        if (info.hbmMask) {
            DeleteObject(info.hbmMask);
        }
        return nullptr;
    }

    HICON icon = CreateIconIndirect(&info);
    DeleteObject(info.hbmColor);
    DeleteObject(info.hbmMask);
    return icon;
}

}
