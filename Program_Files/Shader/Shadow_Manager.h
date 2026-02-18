/*==============================================================================

    Management Shadow Mapping Resources [Shadow_Manager.h]

    Author : Choi HyungJoon

==============================================================================*/
#ifndef SHADOW_MANAGER_H
#define SHADOW_MANAGER_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

constexpr int Shadow_Map_W = 4096;
constexpr int Shadow_Map_H = 4096;

extern float PCF_Spread;
extern int   PCF_Loop;
constexpr float Spread = 1.5f;
constexpr int   Loop   = 2; // 1 = 3x3, 2 = 5x5, 3 = 7x7...

class Shadow_Manager
{
public:
    Shadow_Manager();
    ~Shadow_Manager();

    bool Init(ID3D11Device* device, int width, int height);

    void Begin_Shadow_Pass(ID3D11DeviceContext* context);
    void End_Shadow_Pass(ID3D11DeviceContext* context);

    ID3D11ShaderResourceView* GetShadowMapSRV() const { return m_ShadowMapSRV.Get(); }

    // Get Light View-Matrix
    //    lightDir  : Light Direction (Get Info To Light_Manager)
    //    centerPos : Center Pos For Shadow Map (Always >> Player / Sometime >> Enemy, Cube...etc)
    DirectX::XMMATRIX GetLightViewProjMatrix(const DirectX::XMFLOAT4& lightDir, const DirectX::XMFLOAT3& centerPos);

    void Reset_PCF();
    
private:
    int m_Width = 0;
    int m_Height = 0;

    // Shadow Resources
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_ShadowMapTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   m_ShadowMapDSV; // Use For Draw (Depth Buffer)
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShadowMapSRV; // Use For Read (Texture)

    D3D11_VIEWPORT m_Viewport = {};

    // Resources For Reset
    ID3D11RenderTargetView* m_pOldRTV = nullptr;
    ID3D11DepthStencilView* m_pOldDSV = nullptr;
    D3D11_VIEWPORT          m_OldViewport = {};
};

#endif // SHADOW_MANAGER_H