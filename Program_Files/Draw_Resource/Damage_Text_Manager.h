/*==============================================================================
* 
    Manage Floating Damage Text [Damage_Text_Manager.h]

    Author : Choi HyungJoon

==============================================================================*/

#ifndef DAMAGE_TEXT_MANAGER_H
#define DAMAGE_TEXT_MANAGER_H

#include <vector>
#include <DirectXMath.h>
#include <string>

// Damage Node Structure
struct Damage_Node
{
    bool Active = false;
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f }; // Current Position
    DirectX::XMFLOAT3 Velocity = { 0.0f, 0.0f, 0.0f }; // Pysics Velocity

    int DamageValue = 0;      // Shown Damage Value
    float LifeTime = 0.0f;
    float Max_LifeTime = 0.0f;

    DirectX::XMFLOAT4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    float Scale = 1.0f;
};



class Damage_Text_Manager
{
public:
    static Damage_Text_Manager& GetInstance()
    {
        static Damage_Text_Manager instance;
        return instance;
    }

    void Init();
    void Final();
    void Update(double dt);
    void Draw();
    void Reset();

    void Spawn_Damage(const DirectX::XMFLOAT3& Pos, int Damage,
        const DirectX::XMFLOAT4& Color = { 1.0f, 1.0f, 1.0f, 1.0f },
        float Scale = 1.0f);

private:
    Damage_Text_Manager() = default;
    ~Damage_Text_Manager() = default;

    static const int MAX_DAMAGE_POOL = 200;
    std::vector<Damage_Node> m_DamagePool;
    int m_TexID_Num[10] = { -1 };
    const float GRAVITY = 15.0f;
};
#endif // DAMAGE_TEXT_MANAGER_H