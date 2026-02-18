/*==============================================================================
   
   Manage Enemy Spawning Logic [Enemy_Spawner.cpp]
   
   Author : Choi HyungJoon

==============================================================================*/
#include "Enemy_Spawner.h"
#include "Enemy_Manager.h"
#include "Player.h"
#include "Mash_Field.h"
#include "debug_ostream.h"
#include "Random_Heapler_Logic.h"
using namespace DirectX;

void Enemy_Spawner::Init()
{
    m_GameTime = 0.0f;

    // Timer Reset count >> Wave Count
    m_WaveTimers.clear();
    m_WaveTimers.resize(Wave_Data_Count, 0.0f);
}

void Enemy_Spawner::Reset()
{
    Init();
}

void Enemy_Spawner::Update(double dt)
{
    float fdt = static_cast<float>(dt);
    m_GameTime += fdt;

    // Check Wave Time
    for (int i = 0; i < Wave_Data_Count; ++i)
    {
        const WaveInfo& wave = Wave_Data[i];

        // Is Now Wave Time?
        if (m_GameTime >= wave.StartTime && m_GameTime < wave.EndTime)
        {
            // Timer Reset
            m_WaveTimers[i] -= fdt;

            // Spawn
            if (m_WaveTimers[i] <= 0.0f)
            {
                // Timer Reset For Next Spawn
                m_WaveTimers[i] = wave.SpawnInterval;

                // Spawn For Count
                for (int count = 0; count < wave.BatchCount; ++count)
                {
                    Enemy_Manager::GetInstance().Spawn(wave.Type, Get_Safe_Spawn_Position());
                }
            }
        }
    }
}

XMFLOAT3 Enemy_Spawner::Get_Safe_Spawn_Position()
{
    // 1. Player POS
    XMFLOAT3 playerPos = Player_Get_POS();

    // 2. Random Logic (Range : 25 ~ 250)
    float Angle = RandomFloat(0.0f, XM_2PI);       // 0 ~ 360 Deree
    float Distance = RandomFloat(25.0f, 200.0f);   // Min 25, Max 200

    // 3. Calculate Tentative Position
    float Spawn_X = playerPos.x + cosf(Angle) * Distance;
    float Spawn_Z = playerPos.z + sinf(Angle) * Distance;

	// 4. Boundary Check & Warp Logic (Overload Reset)
    // [X Axis Check]
    if (Spawn_X > Mash_Field_Get_Size_X())
    {
        Spawn_X = -(Spawn_X - Mash_Field_Get_Size_X());
    }
    else if (Spawn_X < -Mash_Field_Get_Size_X())
    {
        Spawn_X = -(Spawn_X + Mash_Field_Get_Size_X());
    }

    // [Z Axis Check]
    if (Spawn_Z > Mash_Field_Get_Size_Z())
    {
        Spawn_Z = -(Spawn_Z - Mash_Field_Get_Size_Z());
    }
    else if (Spawn_Z < -Mash_Field_Get_Size_Z())
    {
        Spawn_Z = -(Spawn_Z + Mash_Field_Get_Size_Z());
    }

    return XMFLOAT3(Spawn_X, 0.0f, Spawn_Z);
}