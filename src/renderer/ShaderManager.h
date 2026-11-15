#pragma once

#include "renderer/GraphicsDevice.h"

#include <string>
#include <unordered_map>

namespace ge {

struct ShaderProgram {
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
};

class ShaderManager {
public:
    explicit ShaderManager(GraphicsDevice& device);

    bool LoadShader(const std::string& id, const std::wstring& path);
    bool Bind(const std::string& id);
    void Unload(const std::string& id);
    void UnloadAll();
    bool Has(const std::string& id) const;

private:
    bool CompileShader(const std::wstring& path, const char* entry, const char* target, ComPtr<ID3DBlob>& blob);

    GraphicsDevice& m_device;
    std::unordered_map<std::string, ShaderProgram> m_shaders;
    ComPtr<ID3D11Buffer> m_fullscreenVB;
    bool EnsureFullscreenQuad();
};

}
