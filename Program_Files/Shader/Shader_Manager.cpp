/*==============================================================================

    Management 2D And 3D Shader Resources [Shader_Manager.cpp]

    Author : Choi HyungJoon

    - Note
        > Make With Singleton Pattern
        > Function Type  : Pointer

==============================================================================*/
#include "Shader_Manager.h"
#include "debug_ostream.h"
#include "Model.h"
#include <fstream>

using namespace DirectX;
using namespace Microsoft::WRL;

Shader_Manager* Shader_Manager::GetInstance()
{
    static Shader_Manager instance;
    return &instance;
}

bool Shader_Manager::Init(ID3D11Device* device, ID3D11DeviceContext* context)
{
    // Force set device/context
    m_device = device;
    m_context = context;

    // --- Initialize 2D Shaders ---
    D3D11_INPUT_ELEMENT_DESC layout2D[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (!loadVertexShader("Resource/Shader/shader_vertex_2d.cso", m_vs2D, m_il2D, layout2D, ARRAYSIZE(layout2D))) return false;
    if (!loadPixelShader("Resource/Shader/shader_pixel_2d.cso", m_ps2D)) return false;

    // --- Initialize 3D Shaders ---
    D3D11_INPUT_ELEMENT_DESC layout3D[] = 
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (!loadVertexShader("Resource/Shader/shader_vertex_3d.cso", m_vs3D, m_il3D, layout3D, ARRAYSIZE(layout3D))) return false;
    if (!loadPixelShader("Resource/Shader/shader_pixel_3d.cso", m_ps3D)) return false;
    if (!loadPixelShader("Resource/Shader/shader_pixel_field.cso", m_ps3D_Field)) return false;

    // --- Initialize Billboard Shaders ---
    D3D11_INPUT_ELEMENT_DESC layoutBillboard[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (!loadVertexShader("Resource/Shader/shader_vertex_Billboard.cso", m_vsBillboard, m_ilBillboard, layoutBillboard, ARRAYSIZE(layoutBillboard))) return false;
    if (!loadPixelShader("Resource/Shader/shader_pixel_Billboard.cso", m_psBillboard)) return false;

    // --- Initialize Skinning Shaders ---
    // Animation Model Need Bone ID And Weight.
    D3D11_INPUT_ELEMENT_DESC layoutSkinning[] =  
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",        0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // Bone Indices
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // Bone Weights
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (!loadVertexShader("Resource/Shader/shader_vertex_Skinning.cso", m_vsSkinning, m_ilSkinning, layoutSkinning, ARRAYSIZE(layoutSkinning))) return false;

    // --- Create Constant Buffers ---
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(XMFLOAT4X4);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.Usage = D3D11_USAGE_DEFAULT; // If Update Every Frame Change To DYNAMIC

    // 2D Buffers (VS)
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbProjection2D);    // b0
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbWorld2D);         // b1

    // 3D Buffers (VS)
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbWorld3D);         // b0
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbView3D);          // b1
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbProjection3D);    // b2

    // PS b0: Diffuse Color
    cbDesc.ByteWidth = sizeof(XMFLOAT4);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbDiffuseColorPS);

    // PS b1: Ambient Light
    cbDesc.ByteWidth = sizeof(XMFLOAT4);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbAmbient3D);

    // PS b2: Directional Light
    cbDesc.ByteWidth = sizeof(Directional_Light);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbDirectional3D);

    // PS b3: Specular Light
    cbDesc.ByteWidth = sizeof(Specular_Light);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbSpecular3D);

    // PS b4: Point Light
    cbDesc.ByteWidth = sizeof(Point_Light_Buffer);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbPointLightPS);

    // PS b5: Shadow Parameters
    cbDesc.ByteWidth = sizeof(Shadow_Parameters);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    m_device->CreateBuffer(&cbDesc, nullptr, &m_cbShadowParams);

    // --- Create Sampler States ---
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    samplerDesc.MaxAnisotropy = 16;

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    m_device->CreateSamplerState(&samplerDesc, &m_sampler_Point);

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    m_device->CreateSamplerState(&samplerDesc, &m_sampler_Linear);

    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    m_device->CreateSamplerState(&samplerDesc, &m_sampler_AnisoTropic);

    SetDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    // --- Create UV Patameter States ---
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.ByteWidth = sizeof(UV_Parameter);

    // --- Create Blend Render ---
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = m_device->CreateBuffer(&bd, nullptr, m_cbBillboardUV.GetAddressOf());
    if (FAILED(hr))
    {
#if defined(DEBUG) || defined(_DEBUG)
        MessageBox(nullptr, "Failed to create UV constant buffer.", "Error", MB_OK);
#else
        MessageBox(nullptr, L"Failed to create UV constant buffer.", L"Error", MB_OK);
#endif
        return false;
    }

    hr = m_device->CreateBlendState(&blendDesc, m_BlendStateAlpha.GetAddressOf());
    if (FAILED(hr))
    {
#if defined(DEBUG) || defined(_DEBUG)
        MessageBox(nullptr, "Failed to create Alpha Blend State", "Error", MB_OK);
#else
        MessageBox(nullptr, L"Failed to create Alpha Blend State", L"Error", MB_OK);
#endif
        return false;
    }

    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    hr = m_device->CreateBlendState(&blendDesc, m_BlendStateOpaque.GetAddressOf());
    if (FAILED(hr))
    {
#if defined(DEBUG) || defined(_DEBUG)
        MessageBox(nullptr, "Failed to create Opaque Blend State", "Error", MB_OK);
#else
        MessageBox(nullptr, L"Failed to create Opaque Blend State", L"Error", MB_OK);
#endif
        return false;
    }

    // --- Create Bone Constant Buffer ---
    D3D11_BUFFER_DESC boneBufferDesc = {};
    boneBufferDesc.ByteWidth = sizeof(Bone_Buffer); // 256 * 64 bytes
    boneBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    boneBufferDesc.Usage = D3D11_USAGE_DEFAULT;     // Will Be Update For Every Frame Or OBJ, So Use DEFAULT or DYNAMIC
    m_device->CreateBuffer(&boneBufferDesc, nullptr, &m_cbBones);

	// ---  Rasterizer State (For Shadow Map) ---
	D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = 0;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.ScissorEnable = FALSE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;

    if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_RS_CullNone.GetAddressOf())))
    {
#if defined(DEBUG) || defined(_DEBUG)
        MessageBox(nullptr, "Failed to create Cull None Mode", "Error", MB_OK);
#else
        MessageBox(nullptr, L"Failed to create Cull None Mode", L"Error", MB_OK);
#endif
        return false;
    }

    rasterDesc.CullMode = D3D11_CULL_BACK;
    if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_RS_CullBack.GetAddressOf())))
    {
#if defined(DEBUG) || defined(_DEBUG)
        MessageBox(nullptr, "Failed to create Cull Back Mode", "Error", MB_OK);
#else
        MessageBox(nullptr, L"Failed to create Cull Back Mode", L"Error", MB_OK);
#endif
        return false;
    }

    return true;
}

void Shader_Manager::Final()
{
	// Use Comptrs For Auto, so just reset
    m_device = nullptr;
    m_context = nullptr;
}

void Shader_Manager::SetAlphaBlend(bool enable)
{
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if (enable)
    {
        m_context->OMSetBlendState(m_BlendStateAlpha.Get(), blendFactor, 0xffffffff);
    }
    else
    {
        m_context->OMSetBlendState(m_BlendStateOpaque.Get(), blendFactor, 0xffffffff);
    }
}

//============================================================
//               --- Methods for 2D Shader ---
//============================================================
void Shader_Manager::Begin2D(Shader_Filter Filter)
{
    m_context->VSSetShader(m_vs2D.Get(), nullptr, 0);
    m_context->PSSetShader(m_ps2D.Get(), nullptr, 0);
    m_context->IASetInputLayout(m_il2D.Get()); // b0, b1

    ID3D11Buffer* cbs[] = { m_cbProjection2D.Get(), m_cbWorld2D.Get() };
    m_context->VSSetConstantBuffers(0, 2, cbs);

    switch (Filter)
    {
    case Shader_Filter::MAG_MIP_POINT:
        m_context->PSSetSamplers(0, 1, m_sampler_Point.GetAddressOf());
        break;

    case Shader_Filter::MAG_MIP_LINEAR:
        m_context->PSSetSamplers(0, 1, m_sampler_Linear.GetAddressOf());
        break;

    case Shader_Filter::ANISOTROPIC:
        m_context->PSSetSamplers(0, 1, m_sampler_AnisoTropic.GetAddressOf());
        break;
    }

    SetAlphaBlend(true);
}

void Shader_Manager::SetProjectionMatrix2D(const XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    m_context->UpdateSubresource(m_cbProjection2D.Get(), 0, nullptr, &transpose, 0, 0);
}

void Shader_Manager::SetWorldMatrix2D(const XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    m_context->UpdateSubresource(m_cbWorld2D.Get(), 0, nullptr, &transpose, 0, 0);
}

void Shader_Manager::SetTexture2D(ID3D11ShaderResourceView* textureView)
{
    m_context->PSSetShaderResources(0, 1, &textureView);
}

//============================================================
//               --- Methods for 3D Shader ---
//============================================================
void Shader_Manager::Begin3D(Shader_Filter Filter)
{
    m_context->VSSetShader(m_vs3D.Get(), nullptr, 0);
    m_context->PSSetShader(m_ps3D.Get(), nullptr, 0);
    m_context->IASetInputLayout(m_il3D.Get());

    // --- VS Buffers (b0, b1, b2) ---
    ID3D11Buffer* vsCbs[] = { m_cbWorld3D.Get(), m_cbView3D.Get(), m_cbProjection3D.Get() };
    m_context->VSSetConstantBuffers(0, 3, vsCbs); // b0, b1, b2

    // --- Get Shadow Buffer ---
    m_context->VSSetConstantBuffers(3, 1, m_cbShadow.GetAddressOf());

    // --- PS Buffers ---
    ID3D11Buffer* psCbs[] = {
        m_cbDiffuseColorPS.Get(), // b0
        m_cbAmbient3D.Get(),      // b1
        m_cbDirectional3D.Get(),  // b2
        m_cbSpecular3D.Get(),     // b3
        m_cbPointLightPS.Get(),    // b4
        m_cbShadowParams.Get()    // b5:
    };
    m_context->PSSetConstantBuffers(0, 6, psCbs); // b0, b1, b2, b3, b4, b5

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(m_context->Map(m_cbPointLightPS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        memcpy(mappedResource.pData, &m_PointLightData, sizeof(Point_Light_Buffer));
        m_context->Unmap(m_cbPointLightPS.Get(), 0);
    }

    switch (Filter)
    {
    case Shader_Filter::MAG_MIP_POINT:
        m_context->PSSetSamplers(0, 1, m_sampler_Point.GetAddressOf());
        break;

    case Shader_Filter::MAG_MIP_LINEAR:
        m_context->PSSetSamplers(0, 1, m_sampler_Linear.GetAddressOf());
        break;

    case Shader_Filter::ANISOTROPIC:
        m_context->PSSetSamplers(0, 1, m_sampler_AnisoTropic.GetAddressOf());
        break;
    }

    // --- Set Shadow Buffer ---
    m_context->PSSetSamplers(1, 1, m_sampler_Linear.GetAddressOf());

    SetAlphaBlend(false);
}

void Shader_Manager::Begin3D_For_Field(Shader_Filter Filter)
{
    m_context->VSSetShader(m_vs3D.Get(), nullptr, 0);       // Use 3D VS
    m_context->PSSetShader(m_ps3D_Field.Get(), nullptr, 0); // PS for Field
    m_context->IASetInputLayout(m_il3D.Get());

    // --- VS Buffers (b0, b1, b2) ---
    ID3D11Buffer* vsCbs[] = { m_cbWorld3D.Get(), m_cbView3D.Get(), m_cbProjection3D.Get() };
    m_context->VSSetConstantBuffers(0, 3, vsCbs); // b0, b1, b2

    // --- Get Shadow Buffer ---
    m_context->VSSetConstantBuffers(3, 1, m_cbShadow.GetAddressOf());

    // --- PS Buffers (b1, b2, b3) ---
    ID3D11Buffer* psCbs[] = {
        m_cbDiffuseColorPS.Get(), // b0: Diffuse Color
        m_cbAmbient3D.Get(),      // b1
        m_cbDirectional3D.Get(),  // b2
        m_cbSpecular3D.Get(),     // b3
		m_cbPointLightPS.Get(),   // b4 << Don't Use For Field
        m_cbShadowParams.Get()    // b5
    };
    m_context->PSSetConstantBuffers(0, 6, psCbs); // b1, b2, b3, b4 (Dummy), b5

    ID3D11Buffer* nullCB = nullptr;
    m_context->PSSetConstantBuffers(4, 1, &nullCB);

    switch (Filter)
    {
    case Shader_Filter::MAG_MIP_POINT:
        m_context->PSSetSamplers(0, 1, m_sampler_Point.GetAddressOf());
        break;

    case Shader_Filter::MAG_MIP_LINEAR:
        m_context->PSSetSamplers(0, 1, m_sampler_Linear.GetAddressOf());
        break;

    case Shader_Filter::ANISOTROPIC:
        m_context->PSSetSamplers(0, 1, m_sampler_AnisoTropic.GetAddressOf());
        break;
    }

    // --- Set Shadow Buffer ---
    m_context->PSSetSamplers(1, 1, m_sampler_Linear.GetAddressOf());

    SetAlphaBlend(false);
}

void Shader_Manager::SetWorldMatrix3D(const XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    m_context->UpdateSubresource(m_cbWorld3D.Get(), 0, nullptr, &transpose, 0, 0);
}

void Shader_Manager::SetViewMatrix3D(const XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    m_context->UpdateSubresource(m_cbView3D.Get(), 0, nullptr, &transpose, 0, 0);
}

void Shader_Manager::SetProjectionMatrix3D(const XMMATRIX& matrix)
{
    XMFLOAT4X4 transpose;
    XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
    m_context->UpdateSubresource(m_cbProjection3D.Get(), 0, nullptr, &transpose, 0, 0);
}

void Shader_Manager::SetTexture3D(ID3D11ShaderResourceView* textureView)
{
    m_context->PSSetShaderResources(0, 1, &textureView);
}

void Shader_Manager::SetFieldTextures(ID3D11ShaderResourceView* texture0, ID3D11ShaderResourceView* texture1)
{
    ID3D11ShaderResourceView* textures[] = { texture0, texture1 };
    m_context->PSSetShaderResources(0, 2, textures);
}

//============================================================
//           --- Methods for Billboard System ---
//============================================================
void Shader_Manager::Begin_Billboard()
{
    m_context->VSSetShader(m_vsBillboard.Get(), nullptr, 0);
    m_context->PSSetShader(m_psBillboard.Get(), nullptr, 0);

    m_context->IASetInputLayout(m_ilBillboard.Get());

    m_context->VSSetConstantBuffers(0, 1, m_cbWorld3D.GetAddressOf());        // VS b0 - World
    m_context->VSSetConstantBuffers(1, 1, m_cbView3D.GetAddressOf());         // VS b1 - View
    m_context->VSSetConstantBuffers(2, 1, m_cbProjection3D.GetAddressOf());   // VS b2 - Projection
    m_context->VSSetConstantBuffers(3, 1, m_cbBillboardUV.GetAddressOf());    // VS b3 - UV

    m_context->PSSetConstantBuffers(0, 1, m_cbDiffuseColorPS.GetAddressOf()); // PS b0 - Diffuse Color

    m_context->PSSetSamplers(0, 1, m_sampler_Linear.GetAddressOf());
}

void Shader_Manager::SetUVParameter(const UV_Parameter& parameter)
{
    m_context->UpdateSubresource(m_cbBillboardUV.Get(), 0, nullptr, &parameter, 0, 0);
}

// PS b0 (Diffuse Color)
void Shader_Manager::SetDiffuseColor(const XMFLOAT4& color)
{
    m_context->UpdateSubresource(m_cbDiffuseColorPS.Get(), 0, nullptr, &color, 0, 0);
}

// PS b1 (Ambient)
void Shader_Manager::SetLightAmbient(const XMFLOAT4& color)
{
    m_context->UpdateSubresource(m_cbAmbient3D.Get(), 0, nullptr, &color, 0, 0);
}

// PS b2 (Directional)
void Shader_Manager::SetLightDirectional(const XMFLOAT4& worldDirection, const XMFLOAT4& color)
{
    Directional_Light Data{};
    Data.Direction = worldDirection;
    Data.Color = color;

    m_context->UpdateSubresource(m_cbDirectional3D.Get(), 0, nullptr, &Data, 0, 0);
}

// PS b3 (Specular)
void Shader_Manager::SetLightSpecular(const DirectX::XMFLOAT3& cameraPosition, float power, const DirectX::XMFLOAT4& color)
{
    Specular_Light Data{};
    Data.CameraPosition = cameraPosition;
    Data.Power = power;
    Data.Color = color;

    m_context->UpdateSubresource(m_cbSpecular3D.Get(), 0, nullptr, &Data, 0, 0);
}

// PS b4 (Point Light)
// Index, { POS }, Range, { R, G, B, Power }
void Shader_Manager::SetPointLight(int index, const DirectX::XMFLOAT3& worldPosition, float range, const DirectX::XMFLOAT4& color)
{
    if (index < 0 || index >= 4) return;
    m_PointLightData.point_light[index].Position = worldPosition;
    m_PointLightData.point_light[index].Range = range;
    m_PointLightData.point_light[index].Color = color;
}

void Shader_Manager::SetPointLightCount(int count)
{
    m_PointLightData.point_light_count = count;
}

//============================================================
//          --- Methods for 3D Model Animation ---
//============================================================
void Shader_Manager::Begin3D_Skinning(Shader_Filter Filter)
{
    // Change Vertex Shader
    m_context->VSSetShader(m_vsSkinning.Get(), nullptr, 0);
    // Reuse Pixel Shader For 3D Lighting
    m_context->PSSetShader(m_ps3D.Get(), nullptr, 0);

    m_context->IASetInputLayout(m_ilSkinning.Get());

    // --- VS Buffers ---
    // b0(World), b1(View), b2(Proj) Is Same To 3D
    ID3D11Buffer* vsCbs[] = { m_cbWorld3D.Get(), m_cbView3D.Get(), m_cbProjection3D.Get(), m_cbBones.Get() };
    // b0 ~ b3 = 4
    m_context->VSSetConstantBuffers(0, 4, vsCbs);

    // --- PS Buffers ---
    // Set Same To 3D P.S
    ID3D11Buffer* psCbs[] = {
        m_cbDiffuseColorPS.Get(), // b0
        m_cbAmbient3D.Get(),      // b1
        m_cbDirectional3D.Get(),  // b2
        m_cbSpecular3D.Get(),     // b3
        m_cbPointLightPS.Get(),    // b4
        m_cbShadowParams.Get()    // b5
    };
    m_context->PSSetConstantBuffers(0, 6, psCbs);

    // Sampler Setting
    switch (Filter)
    {
    case Shader_Filter::MAG_MIP_POINT:
        m_context->PSSetSamplers(0, 1, m_sampler_Point.GetAddressOf());
        break;
    case Shader_Filter::MAG_MIP_LINEAR:
        m_context->PSSetSamplers(0, 1, m_sampler_Linear.GetAddressOf());
        break;
    case Shader_Filter::ANISOTROPIC:
        m_context->PSSetSamplers(0, 1, m_sampler_AnisoTropic.GetAddressOf());
        break;
    }

    SetAlphaBlend(false);
}

void Shader_Manager::SetBones(const DirectX::XMFLOAT4X4* bones, int count)
{
    if (count > 256) count = 256; // Lock Buffer Scale

    static Bone_Buffer cbData = {};
    ZeroMemory(&cbData, sizeof(Bone_Buffer)); // Safety Code For Trash Deta

    // If Bone Working Is Werid, Use XMMatrixTranspose Change
    for (int i = 0; i < count; ++i)
    {
        DirectX::XMMATRIX m = DirectX::XMLoadFloat4x4(&bones[i]);
        DirectX::XMStoreFloat4x4(&cbData.boneTransforms[i], DirectX::XMMatrixTranspose(m));
    }

    m_context->UpdateSubresource(m_cbBones.Get(), 0, nullptr, &cbData, 0, 0);
}

//============================================================
//             --- Methods For Shadow Mapping ---
//============================================================
bool Shader_Manager::InitShadow(int width, int height)
{
    // 1. Shadow Manager Init
    m_ShadowManager = std::make_unique<Shadow_Manager>();
    if (!m_ShadowManager->Init(m_device, width, height)) return false;

    // 2. Input Layout
    D3D11_INPUT_ELEMENT_DESC layoutSkinning[] = // Animation Shadow
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",        0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (!loadVertexShader("Resource/Shader/shader_vertex_shadow_animation.cso", m_vsShadow_Anim, m_ilShadow_Anim, layoutSkinning, ARRAYSIZE(layoutSkinning))) return false;
    
    D3D11_INPUT_ELEMENT_DESC layoutStatic[] = // Static OBJ Shadow
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (!loadVertexShader("Resource/Shader/shader_vertex_shadow.cso", m_vsShadow_Static, m_ilShadow_Static, layoutStatic, ARRAYSIZE(layoutStatic))) return false;

    // Make Shadow Buffer (b0)
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(CB_Shadow_VS); // World + LightVP, 128 bytes
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.Usage = D3D11_USAGE_DEFAULT;

    HRESULT hr = m_device->CreateBuffer(&cbDesc, nullptr, m_cbShadow.GetAddressOf());
    if (FAILED(hr))
    {
        Debug::D_Out << "Failed to Create Shadow Constant Buffer!" << std::endl;
        return false;
    }

    SetShadowQuality(PCF_Spread, PCF_Loop); // Set Default Shadow Quality (Bias, PCF Sample Count)

    return true;
}

void Shader_Manager::FinalShadow()
{
    m_ShadowManager.release();
}

void Shader_Manager::BeginShadow_Skinning()
{
    // 1. Shader Set
    m_context->VSSetShader(m_vsShadow_Anim.Get(), nullptr, 0);
    m_context->PSSetShader(nullptr, nullptr, 0); // Don`t Need Pixel Shader (Just Save Depth)

    // 2. Set Input Layout For Animation
    m_context->IASetInputLayout(m_ilShadow_Anim.Get());

    // 3. Set Buffer
    m_context->VSSetConstantBuffers(0, 1, m_cbShadow.GetAddressOf());   // b0: World + LightVP
    m_context->VSSetConstantBuffers(3, 1, m_cbBones.GetAddressOf());    // b3: Bones

	// 4. Set Rasterizer State To Cull None
    m_context->RSSetState(m_RS_CullNone.Get());

    // 5. Change Render Target To Shadow Map
    m_ShadowManager->Begin_Shadow_Pass(m_context);
}

void Shader_Manager::BeginShadow_Static()
{
    // 1. Set Shader For Static OBJ
    m_context->VSSetShader(m_vsShadow_Static.Get(), nullptr, 0);
    m_context->PSSetShader(nullptr, nullptr, 0); // Turn Off The Pixel Shader

    // 2. Set Layout
    m_context->IASetInputLayout(m_ilShadow_Static.Get());

    // 3. Set Buffer (b0: World + LightVP)
    m_context->VSSetConstantBuffers(0, 1, m_cbShadow.GetAddressOf());

	// 4. Set Rasterizer State To Cull None
    m_context->RSSetState(m_RS_CullNone.Get());
}

void Shader_Manager::EndShadow()
{
    if (m_ShadowManager)
    {
        m_ShadowManager->End_Shadow_Pass(m_context);
    }

	// Restore Rasterizer State To Default
    m_context->RSSetState(m_RS_CullBack.Get());
}

void Shader_Manager::DrawModelShadow(MODEL* model, const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& lightVP)
{
    if (!model) return;

    // 1. Change Input Layout to match Model Buffer (Stride 80)
    m_context->IASetInputLayout(m_ilShadow_Static.Get());

    // 2. Update Matrix (b0)
    SetShadowWorldMatrix(world, lightVP);

    // 3. Define Vertex Struct for Stride
    UINT stride = sizeof(Vertex3D);
    UINT offset = 0;

    // 4. Draw Shadows
    for (unsigned int i = 0; i < model->AiScene->mNumMeshes; ++i)
    {
        ID3D11Buffer* vb = model->VertexBuffer[i];
        ID3D11Buffer* ib = model->IndexBuffer[i];

        // Set Buffers
        m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        m_context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

        // Draw
        m_context->DrawIndexed(model->AiScene->mMeshes[i]->mNumFaces * 3, 0, 0);
    }
}

void Shader_Manager::DrawModelShadow_Animation(MODEL* model, const XMMATRIX& world, const XMMATRIX& lightViewProj)
{
    if (!model) return;

	// 1. Animation Shadow Shader Setting
	// (VS : Animation Shadow, PS : None)
    m_context->VSSetShader(m_vsShadow_Anim.Get(), nullptr, 0);
    m_context->PSSetShader(nullptr, nullptr, 0);

	// 2. Change Input Layout to match Model Buffer (Stride 80)
    m_context->IASetInputLayout(m_ilShadow_Anim.Get());

	// 3-1. Bone Data Update (b3)
    std::vector<XMFLOAT4X4> boneTransforms;
    Model_Get_BoneTransforms(model, boneTransforms);

	// 3-2. Buffer Update For Bones (b3)
    SetBoneTransform(boneTransforms);
    m_context->VSSetConstantBuffers(3, 1, m_cbBones.GetAddressOf());

	// 4. Update World And LightVP Matrix To Shadow Buffer (b0)
    CB_Shadow_VS data = {};
    data.World = XMMatrixTranspose(world);
    data.LightViewProjection = XMMatrixTranspose(lightViewProj);
    m_context->UpdateSubresource(m_cbShadow.Get(), 0, nullptr, &data, 0, 0);
    m_context->VSSetConstantBuffers(0, 1, m_cbShadow.GetAddressOf());

	// 5. Draw Model With Shadow Shader
    UINT stride = sizeof(Vertex3D);
    UINT offset = 0;

    for (size_t i = 0; i < model->AiScene->mNumMeshes; ++i)
    {
        m_context->IASetVertexBuffers(0, 1, &model->VertexBuffer[i], &stride, &offset);
        m_context->IASetIndexBuffer(model->IndexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

        m_context->DrawIndexed(model->AiScene->mMeshes[i]->mNumFaces * 3, 0, 0);
    }
}

void Shader_Manager::SetBoneTransform(const std::vector<XMFLOAT4X4>& transforms)
{
	// Skinning Buffer Struct (b3) - 256 Bones Max
    struct SkinningBufferType
    {
        XMMATRIX bones[256];
    };

    static SkinningBufferType data = {};
    ZeroMemory(&data, sizeof(SkinningBufferType));

	// Safety Check To Avoid Exceeding Shader Array Size (256)
    int count = (int)transforms.size();
    if (count > 256) count = 256;

    // Load Each Bone Matrix And Transpose It For HLSL
    for (int i = 0; i < count; ++i)
    {
        XMMATRIX mtx = XMLoadFloat4x4(&transforms[i]);
        data.bones[i] = XMMatrixTranspose(mtx);
    }

	// Safety Code For Unused Bones
    for (int i = count; i < 256; ++i)
    {
        data.bones[i] = XMMatrixIdentity();
    }

	// Buffer Update (b3 : Bones)
    m_context->UpdateSubresource(m_cbBones.Get(), 0, nullptr, &data, 0, 0);
}

void Shader_Manager::SetShadowWorldMatrix(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& lightViewProj) 
{
    // Update Matrix World And LightVP

    CB_Shadow_VS data = {};
    data.World = XMMatrixTranspose(world);
    data.LightViewProjection = XMMatrixTranspose(lightViewProj);

    m_context->UpdateSubresource(m_cbShadow.Get(), 0, nullptr, &data, 0, 0);
}

void Shader_Manager::SetLightViewProjMatrix(const DirectX::XMMATRIX& lightViewProj)
{
    // Update Matrix Without World

    CB_Shadow_VS data = {};
    data.World = XMMatrixTranspose(XMMatrixIdentity()); // Dummy
    data.LightViewProjection = XMMatrixTranspose(lightViewProj);

    m_context->UpdateSubresource(m_cbShadow.Get(), 0, nullptr, &data, 0, 0);
}

void Shader_Manager::SetShadowMapTexture(ID3D11ShaderResourceView* shadowMapSRV)
{
    m_context->PSSetShaderResources(4, 1, &shadowMapSRV);
}

void Shader_Manager::UnbindShadowMapTexture()
{
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_context->PSSetShaderResources(4, 1, nullSRV);
}

void Shader_Manager::SetShadowQuality(float spread, int loopRange)
{
    if (!m_cbShadowParams) return;

    m_ShadowData.Spread = spread;
    m_ShadowData.LoopRange = loopRange;

	// Update Shadow Quality Data To Shadow Buffer (b5)
    m_context->UpdateSubresource(m_cbShadowParams.Get(), 0, nullptr, &m_ShadowData, 0, 0);
}

void Shader_Manager::ResetShadowQuality()
{
    if (m_ShadowManager)
    {
        m_ShadowManager->Reset_PCF();
    }
}

//============================================================
//              --- Private Helper Methods ---
//============================================================
bool Shader_Manager::loadVertexShader(const char* filename, ComPtr<ID3D11VertexShader>& vs, ComPtr<ID3D11InputLayout>& il, const D3D11_INPUT_ELEMENT_DESC* layout, UINT numElements)
{
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) 
    {
        Debug::D_Out << "Failed to open shader file : " << filename << std::endl;
        return false;
    }

    ifs.seekg(0, std::ios::end);
    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    ifs.read(buffer.data(), size);
    ifs.close();

    HRESULT hr = m_device->CreateVertexShader(buffer.data(), buffer.size(), nullptr, &vs);
    if (FAILED(hr))
    {
        Debug::D_Out << "Failed to create vertex shader : " << filename << std::endl;
        return false;
    }

    hr = m_device->CreateInputLayout(layout, numElements, buffer.data(), buffer.size(), &il);
    if (FAILED(hr))
    {
        Debug::D_Out << "Failed to create input layout for : " << filename << std::endl;
        return false;
    }

    return true;
}

bool Shader_Manager::loadPixelShader(const char* filename, ComPtr<ID3D11PixelShader>& ps)
{
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) 
    {
        Debug::D_Out << "Failed to open shader file : " << filename << std::endl;
        return false;
    }

    ifs.seekg(0, std::ios::end);
    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    ifs.read(buffer.data(), size);
    ifs.close();

    HRESULT hr = m_device->CreatePixelShader(buffer.data(), buffer.size(), nullptr, &ps);
    if (FAILED(hr)) 
    {
        Debug::D_Out << "Failed to create pixel shader : " << filename << std::endl;
        return false;
    }

    return true;
}

