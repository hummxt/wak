#include "renderer/ShaderManager.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include <d3dcompiler.h>
#include <vector>

namespace ge {

struct Vertex {
    float x, y, z;
    float u, v;
};

ShaderManager::ShaderManager(GraphicsDevice& device)
    : m_device(device) {
}

bool ShaderManager::EnsureFullscreenQuad() {
    if (m_fullscreenVB) {
        return true;
    }

    const Vertex vertices[] = {
        { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
        { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
        {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
    };

    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(vertices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = vertices;

    HRESULT hr = m_device.GetDevice()->CreateBuffer(&desc, &data, &m_fullscreenVB);
    return SUCCEEDED(hr);
}

bool ShaderManager::CompileShader(const std::wstring& path, const char* entry, const char* target, ComPtr<ID3DBlob>& blob) {
    ComPtr<ID3DBlob> errors;
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        path.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry,
        target,
        flags,
        0,
        &blob,
        &errors
    );

    if (FAILED(hr)) {
        if (errors) {
            LogError(std::string("Shader compile error: ") + static_cast<const char*>(errors->GetBufferPointer()));
        } else {
            LogError("Shader compile failed: " + Narrow(path));
        }
        return false;
    }
    return true;
}

bool ShaderManager::LoadShader(const std::string& id, const std::wstring& path) {
    if (!EnsureFullscreenQuad()) {
        return false;
    }

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    if (!CompileShader(path, "VSMain", "vs_5_0", vsBlob)) {
        return false;
    }
    if (!CompileShader(path, "PSMain", "ps_5_0", psBlob)) {
        return false;
    }

    ShaderProgram program;
    HRESULT hr = m_device.GetDevice()->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &program.vertexShader
    );
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device.GetDevice()->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &program.pixelShader
    );
    if (FAILED(hr)) {
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_device.GetDevice()->CreateInputLayout(
        layout,
        ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &program.inputLayout
    );
    if (FAILED(hr)) {
        return false;
    }

    m_shaders[id] = std::move(program);
    LogInfo("Loaded shader: " + id);
    return true;
}

bool ShaderManager::Bind(const std::string& id) {
    auto it = m_shaders.find(id);
    if (it == m_shaders.end()) {
        return false;
    }

    auto* context = m_device.GetContext();
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetInputLayout(it->second.inputLayout.Get());
    context->IASetVertexBuffers(0, 1, m_fullscreenVB.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->VSSetShader(it->second.vertexShader.Get(), nullptr, 0);
    context->PSSetShader(it->second.pixelShader.Get(), nullptr, 0);
    return true;
}

void ShaderManager::Unload(const std::string& id) {
    m_shaders.erase(id);
}

void ShaderManager::UnloadAll() {
    m_shaders.clear();
}

bool ShaderManager::Has(const std::string& id) const {
    return m_shaders.find(id) != m_shaders.end();
}

}
