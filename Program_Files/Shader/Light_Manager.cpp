/*==============================================================================

    Management Global Game Light [Light_Manager.h]

    Author : Choi HyungJoon

==============================================================================*/
#include "Light_Manager.h"
#include "Shader_Manager.h"
#include "Palette.h"       

using namespace DirectX;

bool g_IsSunRotation = false;
float g_Sun_Angle = S_Angle, g_Sun_Tilt = S_Tlit, g_Sun_Speed = S_Speed;
XMFLOAT4 g_Sun_Color = C_Son, g_Ambient_Color = C_Ambient;

Light_Manager& Light_Manager::GetInstance()
{
    static Light_Manager instance;
    return instance;
}

void Light_Manager::Init()
{
    Global_Light_Reset();

    // Ambient Light
    // Default : Dark Gray ( 0.1f, 0.1f, 0.1f, 1.0f )
    m_Ambient.Color = g_Ambient_Color;
    m_Directional.Color = g_Sun_Color;

    // Sun Directional (Global Light)
	// Default : Downward Light Gray
    XMVECTOR Dir = XMVectorSet(0.5f, -1.0f, 0.5f, 0.0f);
    Dir = XMVector3Normalize(Dir);
    XMStoreFloat4(&m_Directional.Vector, Dir);


	// Point Lights Initialization
    for (int i = 0; i < 4; i++)
    {
        m_PointLights[i].Position = { 0,0,0 };
        m_PointLights[i].Range = 0.0f;
        m_PointLights[i].Color = { 0,0,0,0 };
    }
    m_PointLightCount = 0;
}

void Light_Manager::Global_Light_Set_Up() const
{
    if (!Shader_Manager::GetInstance()) return;

    Shader_Manager::GetInstance()->SetLightAmbient(m_Ambient.Color);
    Shader_Manager::GetInstance()->SetLightDirectional(m_Directional.Vector, m_Directional.Color);

    Shader_Manager::GetInstance()->SetPointLightCount(m_PointLightCount);
    for (int i = 0; i < 4; i++)
    {
        Shader_Manager::GetInstance()->SetPointLight(i,
            m_PointLights[i].Position,
            m_PointLights[i].Range,
            m_PointLights[i].Color);
    }

    Shader_Manager::GetInstance()->SetDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f });
}

void Light_Manager::Global_Light_Update(double elapsed_time)
{
    float dt = static_cast<float>(elapsed_time);

    if (g_IsSunRotation)
    {
        g_Sun_Angle += g_Sun_Speed * dt;

        // Rotation 360 Degree
        // If Want Teleport, Use "XM_PI"
        if (g_Sun_Angle > DirectX::XM_2PI)
        {
            g_Sun_Angle -= DirectX::XM_2PI;
        }
    }

    // --- Set Global Light ---
    float x = sinf(g_Sun_Angle);
    float y = -cosf(g_Sun_Angle);
    float z = g_Sun_Tilt;

    Set_Directional_Light({ x, y, z, 0.0f }, g_Sun_Color);
    Set_Ambient_Color(g_Ambient_Color);
}

// --- Setters ---
void Light_Manager::Set_Ambient_Color(const DirectX::XMFLOAT4& color)
{
    m_Ambient.Color = color;
}

void Light_Manager::Set_Directional_Light(const DirectX::XMFLOAT4& dir, const DirectX::XMFLOAT4& color)
{
    XMVECTOR Dir = XMLoadFloat4(&dir);
    Dir = XMVector3Normalize(Dir);
    XMStoreFloat4(&m_Directional.Vector, Dir);

    m_Directional.Color = color;
}

void Light_Manager::Set_Point_Light(int index, const DirectX::XMFLOAT3& pos, float range, const DirectX::XMFLOAT4& color)
{
    if (index < 0 || index >= 4) return;
    m_PointLights[index].Position = pos;
    m_PointLights[index].Range = range;
    m_PointLights[index].Color = color;
}

void Light_Manager::Set_Point_Light_Active_Count(int count)
{
    m_PointLightCount = count;
}

void Light_Manager::Global_Light_Reset()
{
    g_IsSunRotation = false;

    g_Sun_Angle = S_Angle;
    g_Sun_Tilt = S_Tlit;
    g_Sun_Speed = S_Speed;

    g_Sun_Color = C_Son;
    g_Ambient_Color = C_Ambient;

    // Reset
    m_Ambient.Color = g_Ambient_Color;

    XMVECTOR Dir = XMVectorSet(sinf(g_Sun_Angle), -cosf(g_Sun_Angle), g_Sun_Tilt, 0.0f);
    Dir = XMVector3Normalize(Dir);
    XMStoreFloat4(&m_Directional.Vector, Dir);

    m_Directional.Color = g_Sun_Color;
}
