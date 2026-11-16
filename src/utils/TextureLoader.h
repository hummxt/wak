#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <wrl/client.h>

namespace ge {

using Microsoft::WRL::ComPtr;

bool LoadTextureFromFile(
    ID3D11Device* device,
    const std::wstring& path,
    ComPtr<ID3D11ShaderResourceView>& outSrv,
    int* outWidth = nullptr,
    int* outHeight = nullptr
);

// Loads an icon with optional content padding and rounded-corner mask (0–1 of size).
HICON LoadIconFromFile(
    const std::wstring& path,
    int width,
    int height,
    float paddingRatio = 0.12f,
    float cornerRadiusRatio = 0.22f
);

}
