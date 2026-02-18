/*==============================================================================

	Manage Enemy Spawn System [Enemy_Manager.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "Enemy_Manager.h"
#include "Enemy_Ground.h"
#include "Enemy_Flight.h"
#include "Player.h"        
#include "Player_Camera.h" 
#include "Mash_Field.h"
#include "Random_Heapler_Logic.h"

// Set pool sizes1
static constexpr int POOL_SIZE_GROUND = 100;
static constexpr int POOL_SIZE_FLIGHT = 100;

void Enemy_Manager::Init()
{
    m_EnemyPool.clear();
    m_ActiveList.clear();

    // Ground Type
    for (int i = 0; i < POOL_SIZE_GROUND; ++i)
    {
        m_EnemyPool.push_back(new Enemy_Ground({ 0,0,0 }));
    }

	// Flight Type
    for (int i = 0; i < POOL_SIZE_FLIGHT; ++i)
    {
        m_EnemyPool.push_back(new Enemy_Flight({ 0,0,0 }));
    }
}

void Enemy_Manager::Final()
{
    for (Enemy* e : m_EnemyPool)
    {
        delete e;
    }

    m_EnemyPool.clear();
    m_ActiveList.clear();
}

void Enemy_Manager::Update(double elapsed_time)
{
    float dt = static_cast<float>(elapsed_time);

    // Update active list
    m_ActiveList.clear();

    for (Enemy* e : m_EnemyPool)
    {
        if (e->IsActive())
        {
            e->Update(dt);
            m_ActiveList.push_back(e);
        }
    }
}

void Enemy_Manager::Draw()
{
    for (Enemy* e : m_ActiveList)
    {
        e->Draw();
    }
}

void Enemy_Manager::Draw_Shadow(const DirectX::XMMATRIX& LightViewProj)
{
    for (Enemy* e : m_ActiveList)
    {
        e->Draw_Shadow(LightViewProj);
    }
}

void Enemy_Manager::Reset()
{
    m_ActiveList.clear();

    for (Enemy* e : m_EnemyPool)
    {
        if (e->IsActive())
        {
            e->Deactivate();
        }
    }
}

void Enemy_Manager::Spawn(EnemyType type, const DirectX::XMFLOAT3& spawnPos)
{
    // Get Player POS, Camera Info
    DirectX::XMFLOAT3 Pos = spawnPos;

    float Ground_Y = Mash_Field_Get_Height(Pos.x, Pos.z);

	if (Ground_Y < -100.0f) // If Ground (Y) Is Too Low, Set Player Y Axis As Ground Y
    {
        Ground_Y = Player_Get_POS().y;
    }

    // Filght Type Y Axis Will Be Update For Enemy Manager, So Just Spawn
    if (type == EnemyType::FLIGHT_NORMAL || type == EnemyType::FLIGHT_DASHER)
    {
        Pos.y = Ground_Y + 10.0f;
    }
    else
    {
		// Safe Spawn Position Check
        Pos.y = Ground_Y + 1.0f;
    }

    // Is Ground Enemy?
    bool isGroundRequest = (type == EnemyType::GROUND_NORMAL ||
        type == EnemyType::GROUND_DASHER ||
        type == EnemyType::GROUND_TANKER);

    // Search for an inactive enemy of the requested type
    for (Enemy* e : m_EnemyPool)
    {
        // Enemy Is Already Spawn, Will Be Skip
        if (e->IsActive())
        {
            continue;
        }

        // Check Enemy Type
        bool isGroundObj = (dynamic_cast<Enemy_Ground*>(e) != nullptr);

        // If Type Is Ground, Return
        if (isGroundRequest == isGroundObj)
        {
            e->Activate(Pos, type);
            return;
        }
    }
    // No available enemy found in the pool
}

Enemy* Enemy_Manager::Check_Collision_AABB(const AABB& box)
{
    for (Enemy* e : m_ActiveList)
    {
        if (Collision_Is_Hit_AABB(box, e->GetAABB()).Is_Hit)
        {
            return e;
        }
    }
    return nullptr;
}