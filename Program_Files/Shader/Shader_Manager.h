/*==============================================================================

    Management 2D And 3D Shader Resources [Shader_Manager.h]

    Author : Choi HyungJoon

==============================================================================*/
#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h> // ComPtr
#include "Shadow_Manager.h"
#include <memory>
#include <vector>

enum class Shader_Filter
{
    MAG_MIP_POINT,   // No filtering (Pixelated look)
    MAG_MIP_LINEAR,  // Smooth bilinear filtering
    ANISOTROPIC      // High-quality filtering for textures at sharp angles
};

struct Directional_Light
{
    DirectX::XMFLOAT4 Direction;
    DirectX::XMFLOAT4 Color;
};

struct Specular_Light
{
    DirectX::XMFLOAT3 CameraPosition;
    float Power;
    DirectX::XMFLOAT4 Color;
};

struct Point_Light
{
    DirectX::XMFLOAT3 Position;
    float Range;
    DirectX::XMFLOAT4 Color;
};

struct Point_Light_Buffer
{
    Point_Light point_light[4];
    int point_light_count;
    DirectX::XMFLOAT3 dummy;
};

struct UV_Parameter
{
    DirectX::XMFLOAT2 scale;
    DirectX::XMFLOAT2 translation;
};

struct Bone_Buffer
{
    DirectX::XMFLOAT4X4 boneTransforms[256]; // Warning : Must Be Same To Skinning V.S Scale
};

struct Shadow_Parameters
{
    float Spread;               // PCF Spread (0.0f ~ 1.0f) : 0.0f = Sharp Shadow, 1.0f = Soft Shadow
    int   LoopRange;            // PCF Loop Count (1 ~ 5) : 1 = Sharp Shadow, 5 = Soft Shadow
    DirectX::XMFLOAT2 Padding;  // Set 16Byte Alignment
};

struct MODEL;

class Shader_Manager
{
public:
    static Shader_Manager* GetInstance();

    Shader_Manager(const Shader_Manager&) = delete;
    Shader_Manager& operator=(const Shader_Manager&) = delete;

    // Initializes all shaders
    bool Init(ID3D11Device* device, ID3D11DeviceContext* context);
    void Final();

    // --- Set Alpha ---
    void SetAlphaBlend(bool enable);

    // --- Methods for 2D Shader ---
    void Begin2D(Shader_Filter Filter = Shader_Filter::MAG_MIP_POINT);

    void SetWorldMatrix2D(const DirectX::XMMATRIX& matrix);
    void SetProjectionMatrix2D(const DirectX::XMMATRIX& matrix);
    void SetTexture2D(ID3D11ShaderResourceView* textureView);

    // --- Methods for 3D Shader ---
    void Begin3D(Shader_Filter Filter = Shader_Filter::ANISOTROPIC);
    void Begin3D_For_Field(Shader_Filter Filter = Shader_Filter::ANISOTROPIC);

    void SetWorldMatrix3D(const DirectX::XMMATRIX& matrix);
    void SetViewMatrix3D(const DirectX::XMMATRIX& matrix);
    void SetProjectionMatrix3D(const DirectX::XMMATRIX& matrix);

    void SetTexture3D(ID3D11ShaderResourceView* textureView);
    void SetFieldTextures(ID3D11ShaderResourceView* texture0, ID3D11ShaderResourceView* texture1); // For Multy Texture

    // --- Methods for Billboard System ---
    void Begin_Billboard();
    void SetUVParameter(const UV_Parameter& parameter);

    void SetDiffuseColor(const DirectX::XMFLOAT4& color);                                                               // Shader b0
    void SetLightAmbient(const DirectX::XMFLOAT4& color);                                                               // Shader b1
    void SetLightDirectional(const DirectX::XMFLOAT4& worldDirection, const DirectX::XMFLOAT4& color);                  // Shader b2
    void SetLightSpecular(const DirectX::XMFLOAT3& cameraPosition, float power, const DirectX::XMFLOAT4& color);        // Shader b3
    void SetPointLight(int index, const DirectX::XMFLOAT3& worldPosition, float range, const DirectX::XMFLOAT4& color); // Shader b4
    void SetPointLightCount(int count);

    // --- Methods for 3D Model Animation ---
    void Begin3D_Skinning(Shader_Filter Filter = Shader_Filter::ANISOTROPIC);
    void SetBones(const DirectX::XMFLOAT4X4* bones, int count); // Send Bone Data

    // --- Methods For Shadow Mapping ---
    bool InitShadow(int width = Shadow_Map_W, int height = Shadow_Map_H);
    void FinalShadow();

    // Manage Shadow Init / Final
    void BeginShadow_Skinning();    // Shadow For Animation
    void BeginShadow_Static();      // Shadow For Static Object
    void EndShadow();               // Return To Draw Pass

    // Helper Logic For Model Shadow Draw
    void DrawModelShadow(MODEL* model, const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& lightVP);
    
	// Helper Logic For Animation Model Shadow Draw (Use Skinning Shader)
    void DrawModelShadow_Animation(MODEL* model, const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& lightViewProj);

	// Send Bone Data To Shadow Shader (Use For Animation Shadow)
    void SetBoneTransform(const std::vector<DirectX::XMFLOAT4X4>& transforms);

    // Send World, Lighting Matrix To Shader
    void SetShadowWorldMatrix(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& lightViewProj);

    // Update Light View-Projection In Main Game Logic
    void SetLightViewProjMatrix(const DirectX::XMMATRIX& lightViewProj);

    // Send Shadow Map Texture To Pixel Shader
    void SetShadowMapTexture(ID3D11ShaderResourceView* shadowMapSRV);

    // Unbind Shadow Map
    void UnbindShadowMapTexture();

	// Set Shadow Quality In GUI (PCF Parameters)
    void SetShadowQuality(float spread, int loopRange);
    void ResetShadowQuality();

    // Getter
    Shadow_Manager* GetShadowManager() { return m_ShadowManager.get(); }

private:
    Shader_Manager() = default;
    ~Shader_Manager() = default;

    // Helper method to load shaders from .cso files
    bool loadVertexShader(const char* filename,
        Microsoft::WRL::ComPtr<ID3D11VertexShader>& vs,
        Microsoft::WRL::ComPtr<ID3D11InputLayout>& il,
        const D3D11_INPUT_ELEMENT_DESC* layout, UINT numElements);

    bool loadPixelShader(const char* filename, Microsoft::WRL::ComPtr<ID3D11PixelShader>& ps);

    // Common device interfaces
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;

    Microsoft::WRL::ComPtr<ID3D11BlendState> m_BlendStateAlpha;  // Alpha Blend
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_BlendStateOpaque; // No Blend

    // --- 2D Shader Resources ---
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs2D;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_il2D;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_ps2D;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbProjection2D; // b0 for projection matrix
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbWorld2D;      // b1 for world matrix

    // --- 3D Shader Resources ---
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs3D;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_il3D;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_ps3D;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_ps3D_Field;      // Pixel Shader For Field

    // --- Billboard Shader Resources ---
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsBillboard;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_ilBillboard;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_psBillboard;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbBillboardUV;     // VS b3 for UV Parameter

    // --- Vertex Shader Resources ---
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbWorld3D;         // VS b0 for World
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbView3D;          // VS b1 for View
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbProjection3D;    // VS b2 for Projection

    // --- Pixel Shader Resources ---
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbDiffuseColorPS;  // PS b0 for Diffuse Light
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbAmbient3D;       // PS b1 for Ambient Light
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbDirectional3D;   // PS b2 for Directional Light
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbSpecular3D;      // PS b3 for Specular Light
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbPointLightPS;    // PS b4 for Point Light

    Point_Light_Buffer m_PointLightData = {};

    // Sampler states for different filtering modes
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler_Point;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler_Linear;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler_AnisoTropic;

    // --- 3D Model Animation Shader Resources ---
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsSkinning;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_ilSkinning;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbBones;       // VS b3

    // --- Shadow Resources ---
    std::unique_ptr<Shadow_Manager> m_ShadowManager; // Shadow Manager Instance

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsShadow_Anim;     // VS For Animation Shadow
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_ilShadow_Anim;	    // Layout For Animation Shadow Shader

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsShadow_Static;   // VS For Static OBJ Shadow
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_ilShadow_Static;   // Layout For Shader

    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_cbShadow;          // VS b0 For Shader

	// Rasterizer States For Shadow Pass
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RS_CullNone;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RS_CullBack;

	// Shadow Quality Parameters For GUI
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbShadowParams;          // PS b5 for Shadow Parameters
    Shadow_Parameters m_ShadowData = { 1.0f, 1, {0,0} };

    // Const Struct For Shadow Shader (b0)
    // >> Alert <<
    // Must Same To Vertex Shader Code
    struct CB_Shadow_VS
    {
        DirectX::XMMATRIX World;
        DirectX::XMMATRIX LightViewProjection;
    };
};

#endif // SHADER_MANAGER_H

