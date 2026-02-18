/*==============================================================================

    Management Shadow Mapping Resources [Shadow_Manager.h]

    Author : Choi HyungJoon

==============================================================================*/
#include "Shadow_Manager.h"
#include <cmath>

using namespace DirectX;

float PCF_Spread = Spread;
int   PCF_Loop = Loop;

Shadow_Manager::Shadow_Manager() {}
Shadow_Manager::~Shadow_Manager() {}

bool Shadow_Manager::Init(ID3D11Device* device, int width, int height)
{
    m_Width = width, m_Height = height;
    PCF_Spread = Spread, PCF_Loop = Loop;

    // --- Set Texture For Depth Map ---
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;

    // >> Alart <<
    // It Will Be Use Shader Resources (R), Depth Stencil (W)
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.Usage = D3D11_USAGE_DEFAULT;

    // Safety Check
    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, m_ShadowMapTexture.GetAddressOf())))  return false;

    // --- Make Depth Stencil View (DSV) For Drawing ---
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Set Depth Pormat
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    // Safety Check
    if (FAILED(device->CreateDepthStencilView(m_ShadowMapTexture.Get(), &dsvDesc, m_ShadowMapDSV.GetAddressOf())))  return false;

    // --- Make Shader Resource View (SRV) For Reading ---
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // Save Depth In Red Channel (R24)
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    // Safety Check
    if (FAILED(device->CreateShaderResourceView(m_ShadowMapTexture.Get(), &srvDesc, m_ShadowMapSRV.GetAddressOf())))    return false;

    // --- Set View Port ---
    m_Viewport.Width    = static_cast<float>(width);
    m_Viewport.Height   = static_cast<float>(height);
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;

    return true;
}

void Shadow_Manager::Begin_Shadow_Pass(ID3D11DeviceContext* context)
{
    // Backup Render Target And ViewPort
    context->OMGetRenderTargets(1, &m_pOldRTV, &m_pOldDSV);
    UINT numViewports = 1;
    context->RSGetViewports(&numViewports, &m_OldViewport);

    // Shadow Is Don`t Need Color, So Change Depth Buffer(DSV) Only, Render Targer Is Null
    ID3D11RenderTargetView* nullRTV = nullptr;
    context->OMSetRenderTargets(1, &nullRTV, m_ShadowMapDSV.Get());

    // Clear Depth Buffer
    context->ClearDepthStencilView(m_ShadowMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Change ViewPort Size To Shadow Map 
    context->RSSetViewports(1, &m_Viewport);
}

void Shadow_Manager::End_Shadow_Pass(ID3D11DeviceContext* context)
{
    // Reset Render Target And Depth Buffer
    context->OMSetRenderTargets(1, &m_pOldRTV, m_pOldDSV);

    // Reset ViewPort
    context->RSSetViewports(1, &m_OldViewport);

    // Manage Reference Count
    if (m_pOldRTV) { m_pOldRTV->Release(); m_pOldRTV = nullptr; }
    if (m_pOldDSV) { m_pOldDSV->Release(); m_pOldDSV = nullptr; }
}

XMMATRIX Shadow_Manager::GetLightViewProjMatrix(const XMFLOAT4& lightDir, const XMFLOAT3& centerPos)
{
    // --- Set Light Pos ---
    // In Now, Player Will Be Center, And Light Is Far Away
    XMVECTOR Light_Dir = XMLoadFloat4(&lightDir);
    XMVECTOR Shadow_Center = XMLoadFloat3(&centerPos);

    // Light Pos = Player Pos - (Light Direction * Distance)
    // Distance Is Moderately Far (e.g. 50.0f, 100.0f...etc)
    XMVECTOR Light_POS = Shadow_Center - (Light_Dir * 100.0f);

    // If Light Is vertical, Change Up Vector Z Axis To 1
    XMVECTOR UpVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    if (std::abs(lightDir.y) > 0.99f)
    {
        UpVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    }

    // --- Get Light View Matrix ---
    // Matrix For Looki Player From Light POS
    XMMATRIX light_View = XMMatrixLookAtLH(Light_POS, Shadow_Center, UpVector);

    // --- Light Projection Matrix ---
    // Sun Light (Directional Light) Is Use Orthographic For Make Shadows Parallel
    // Range (Width, Height) Is Shadow Range
    float Scene_W  = 150.0f, Scene_H = 150.0f; // Shadow Range Of Around Player
    float Near_Z = 1.0f;
    float Far_Z = 500.0f; // How Deep From The Light POS

    XMMATRIX lightProj = XMMatrixOrthographicLH(Scene_W, Scene_H, Near_Z, Far_Z);

    return light_View * lightProj;
}

void Shadow_Manager::Reset_PCF()
{
    PCF_Spread = Spread;
    PCF_Loop = Loop;
}
