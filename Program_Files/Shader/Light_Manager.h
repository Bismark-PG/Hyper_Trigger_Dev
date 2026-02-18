/*==============================================================================

    Management Global Game Light [Light_Manager.h]

    Author : Choi HyungJoon

==============================================================================*/
#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <d3d11.h>
#include <DirectXMath.h>

extern bool g_IsSunRotation;
extern float g_Sun_Angle, g_Sun_Tilt, g_Sun_Speed, g_Sun_Dist;            
extern DirectX::XMFLOAT3 g_Sun_Dir;
extern DirectX::XMFLOAT4 g_Sun_Color;
extern DirectX::XMFLOAT4 g_Ambient_Color;

constexpr float S_Angle = 0.0f, S_Tlit = 0.05f, S_Speed = 0.1f, S_Dist = 100.0f;
constexpr DirectX::XMFLOAT3 S_Dir = { -0.25f, -1.0f, 0.01f };
constexpr DirectX::XMFLOAT4 C_Son = { 0.8f, 0.8f, 0.8f, 1.0f };
constexpr DirectX::XMFLOAT4 C_Ambient = { 0.1f, 0.1f, 0.1f, 1.0f };

class Light_Manager
{
public:
    static Light_Manager& GetInstance();

    Light_Manager(const Light_Manager&) = delete;
    Light_Manager& operator=(const Light_Manager&) = delete;

    void Init();

    void Global_Light_Set_Up() const;
    void Global_Light_Update(double elapsed_time);

    // --- Setters ---
    void Set_Ambient_Color(const DirectX::XMFLOAT4& color);
    void Set_Directional_Light(const DirectX::XMFLOAT4& dir, const DirectX::XMFLOAT4& color);

	// --- Point Light Management ---
    void Set_Point_Light(int index, const DirectX::XMFLOAT3& pos, float range, const DirectX::XMFLOAT4& color);
    void Set_Point_Light_Active_Count(int count);

    // --- Getters ---
    DirectX::XMFLOAT4 Get_Directional_Vector() const { return m_Directional.Vector; }
    DirectX::XMFLOAT4 Get_Directional_Color() const { return m_Directional.Color; }

    void Global_Light_Reset();

private:
    Light_Manager()
        : m_PointLightCount(0)
    {
        m_Ambient.Color = { 0.1f, 0.1f, 0.1f, 1.0f };
        m_Directional.Vector = { 0.0f, -1.0f, 0.0f, 0.0f };
        m_Directional.Color = { 1.0f, 1.0f, 1.0f, 1.0f };

        for (int i = 0; i < 4; ++i)
        {
            m_PointLights[i].Position = { 0.0f, 0.0f, 0.0f };
            m_PointLights[i].Range = 0.0f;
            m_PointLights[i].Color = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }
    ~Light_Manager() = default;

    struct AmbientData
    {
        DirectX::XMFLOAT4 Color;
    };

    struct DirectionalData
    {
		DirectX::XMFLOAT4 Vector; // Direction
		DirectX::XMFLOAT4 Color;  // Value
    };

    struct PointLightData
    {
        DirectX::XMFLOAT3 Position;
        float Range;
        DirectX::XMFLOAT4 Color;
    };

    AmbientData      m_Ambient;
    DirectionalData  m_Directional;
    PointLightData   m_PointLights[4];
    int              m_PointLightCount = 0;
};

#endif // LIGHT_MANAGER_H