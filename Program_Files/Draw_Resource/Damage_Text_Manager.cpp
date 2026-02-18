/*==============================================================================

    Manage Floating Damage Text [Damage_Text_Manager.cpp]

    Author : Choi HyungJoon

==============================================================================*/
#include "Damage_Text_Manager.h"
#include "Texture_Manager.h"
#include "Billboard.h"
#include "Player_Camera.h"
#include "Random_Heapler_Logic.h"
#include "Shader_Manager.h"
#include <string>
#include <cmath>

using namespace DirectX;

void Damage_Text_Manager::Init()
{
    m_DamagePool.clear();
    m_DamagePool.resize(MAX_DAMAGE_POOL);
    for (auto& node : m_DamagePool) node.Active = false;

    m_TexID_Num[0] = Texture_Manager::GetInstance()->GetID("UI_Num_MIN");
    for (int i = 1; i <= 9; ++i)
    {
        std::string texName = "UI_Num_" + std::to_string(i);
        m_TexID_Num[i] = Texture_Manager::GetInstance()->GetID(texName);
    }
}

void Damage_Text_Manager::Final()
{
    m_DamagePool.clear();
}

void Damage_Text_Manager::Reset()
{
    for (auto& node : m_DamagePool) node.Active = false;
}

void Damage_Text_Manager::Spawn_Damage(const DirectX::XMFLOAT3& Pos, int Damage, const DirectX::XMFLOAT4& Color, float Scale)
{
    for (auto& node : m_DamagePool)
    {
        if (!node.Active)
        {
            node.Active = true;
            node.Position = Pos;

            // [Physics]
			// 1. Power Of Upward Force
            float UpForce = RandomFloatRange(3.0f, 6.0f);

			// 2. Power Of Spread Force
            float SpreadForce = RandomFloatRange(1.0f, 2.5f);
            float Angle = RandomFloat(0.0f, XM_2PI);

            node.Velocity.x = cosf(Angle) * SpreadForce;
            node.Velocity.y = UpForce;
            node.Velocity.z = sinf(Angle) * SpreadForce;

            node.DamageValue = Damage;
            node.Color = Color;
            node.Scale = Scale;

			// Set LifeTime
            node.Max_LifeTime = RandomFloatRange(0.8f, 1.2f);
            node.LifeTime = node.Max_LifeTime;

            return;
        }
    }
}

void Damage_Text_Manager::Update(double dt)
{
    float fdt = static_cast<float>(dt);

    for (auto& node : m_DamagePool)
    {
        if (!node.Active) continue;

		// 1. LifeTime Decrease
        node.LifeTime -= fdt;
        if (node.LifeTime <= 0.0f)
        {
            node.Active = false;
            continue;
        }

		// 2. Pysics Update (Gravity)
        node.Velocity.y -= GRAVITY * fdt;

		// 3. Update Position (P = P + Vt)
        XMVECTOR vPos = XMLoadFloat3(&node.Position);
        XMVECTOR vVel = XMLoadFloat3(&node.Velocity);
        vPos += vVel * fdt;
        XMStoreFloat3(&node.Position, vPos);

		// 4. Alpha Fade Out (LifeTime Ratio)
        float alphaRatio = node.LifeTime / (node.Max_LifeTime * 0.5f);
        if (alphaRatio > 1.0f) alphaRatio = 1.0f;
        node.Color.w = alphaRatio;
    }
}

void Damage_Text_Manager::Draw()
{
	// Alpha Blending Enable
    Shader_Manager::GetInstance()->SetAlphaBlend(true);

	// Get Camera Right Vector For Billboard Alignment
    XMFLOAT4X4 viewMtx = Player_Camera_Get_View_Matrix();
    XMMATRIX mView = XMLoadFloat4x4(&viewMtx);
	// If View Matrix = (R^T, U^T, L^T, P) Then Inverse(View) = (R, U, L, -P)
    // View Matrix = (R^T, U^T, L^T)
	// DirectX Uses Row-Major Order, So Right Vector From Row 0
    // Row 0: Right.x, Right.y, Right.z

    XMVECTOR vCameraRight = XMVectorSet(viewMtx._11, viewMtx._21, viewMtx._31, 0.0f);
    vCameraRight = XMVector3Normalize(vCameraRight);

    for (const auto& node : m_DamagePool)
    {
        if (!node.Active) continue;

        std::string sDmg = std::to_string(node.DamageValue);

		// Get Character Width And Total Width
        float Char_Width = node.Scale * 0.6f;
        float Total_Width = (sDmg.length() - 1) * Char_Width;

		// Get Start Position For First Character
        // StartPos = NodePos - (RightVector * (TotalWidth / 2))
        XMVECTOR vStartPos = XMLoadFloat3(&node.Position) - (vCameraRight * (Total_Width * 0.5f));

        // Draw
        for (size_t i = 0; i < sDmg.length(); ++i)
        {
            int digit = sDmg[i] - '0';
            if (digit < 0 || digit > 9) continue;

			// Get Now Character Draw Position
            XMVECTOR vDrawPos = vStartPos + (vCameraRight * (i * Char_Width));
            XMFLOAT3 drawPos;
            XMStoreFloat3(&drawPos, vDrawPos);

			// Draw Billboard With Pivot Middle {0.5, 0.5}, Color, Alpha
            Billboard_Draw(m_TexID_Num[digit], drawPos, node.Scale, node.Scale, { 0.5f, 0.5f }, node.Color);
        }
    }
}