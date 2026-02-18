/*==============================================================================

    Management Global Game Light [Light_Manager.h]

    Author : Choi HyungJoon

==============================================================================*/
#include "Light_Manager.h"
#include "Shader_Manager.h"
#include "Palette.h"       

using namespace DirectX;

bool g_IsSunRotation = false;
float g_Sun_Angle = S_Angle, g_Sun_Tilt = S_Tlit, g_Sun_Speed = S_Speed, g_Sun_Dist = S_Dist;
XMFLOAT3 g_Sun_Dir = S_Dir;
XMFLOAT4 g_Sun_Color = C_Son;
XMFLOAT4 g_Ambient_Color = C_Ambient;

Light_Manager& Light_Manager::GetInstance()
{
    static Light_Manager instance;
    return instance;
}

void Light_Manager::Init()
{
    Global_Light_Reset();

    // Set Ambient, Sun Light Color
    // Default : Dark Gray ( 0.1f, 0.1f, 0.1f, 1.0f )
    m_Ambient.Color = g_Ambient_Color;
    m_Directional.Color = g_Sun_Color;

    // Set Sun Light Direction
	// Default : Top Down
    Set_Directional_Light({ g_Sun_Dir.x, g_Sun_Dir.y, g_Sun_Dir.z, 0.0f }, g_Sun_Color);

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

        g_Sun_Dir.x = sinf(g_Sun_Angle);
        g_Sun_Dir.y = -cosf(g_Sun_Angle);
        g_Sun_Dir.z = g_Sun_Tilt;
    }

    // --- Set Global Light ---
    Set_Directional_Light({ g_Sun_Dir.x, g_Sun_Dir.y, g_Sun_Dir.z, 0.0f }, g_Sun_Color);
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

    g_Sun_Dist = S_Dist;
    g_Sun_Dir = S_Dir;

    g_Sun_Color = C_Son;

    g_Ambient_Color = C_Ambient;

    // Reset
    m_Ambient.Color = g_Ambient_Color;

    XMVECTOR Dir = XMVectorSet(sinf(g_Sun_Angle), -cosf(g_Sun_Angle), g_Sun_Tilt, 0.0f);
    Dir = XMVector3Normalize(Dir);
    XMStoreFloat4(&m_Directional.Vector, Dir);

    m_Directional.Color = g_Sun_Color;
}
