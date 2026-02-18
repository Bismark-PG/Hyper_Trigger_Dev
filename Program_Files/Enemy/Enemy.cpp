/*==============================================================================

	Manage Enemy [Enemy.h]

	Author : Choi HyungJoon

==============================================================================*/
#include "Enemy.h"
#include "Enemy_Manager.h"
#include "Enemy_Ground.h"
#include "Enemy_Flight.h"
#include "Game_Model.h"
#include "Player.h"
#include "Map_System.h"
#include "Debug_Collision.h"
#include <vector>
#include "Game.h"
#include "Resource_Manager.h"
#include "Billboard_Manager.h"
#include "Shader_Manager.h"
#include "Audio_Manager.h"
#include "Upgrade_System.h"  
#include "Weapon_System.h"   
#include "Random_Heapler_Logic.h"
#include "Damage_Text_Manager.h"
using namespace DirectX;

// Enemy Class Implementation
Enemy::Enemy(const XMFLOAT3& pos)
	: Position(pos)
	, m_Info{}
{
	m_pModel = Model_Manager::GetInstance()->GetModel("Ball");
	// Info Will Be Paste In Activate Logic
}

Enemy::~Enemy()
{
}

void Enemy::Activate(const XMFLOAT3& pos, EnemyType type)
{
	m_IsActive = true;
	Position = pos;
	m_State = EnemyState::CHASE;
	m_Gravity = 0.0f;
	m_StateTimer = 0.0f;

	switch (type)
	{
	case EnemyType::GROUND_NORMAL:
		m_Info = Ground_Normal_Info; 
		break;

	case EnemyType::GROUND_DASHER: 
		m_Info = Ground_Dasher_Info; 
		break;

	case EnemyType::GROUND_TANKER: 
		m_Info = Ground_Tanker_Info;
		break;

	case EnemyType::FLIGHT_NORMAL: 
		m_Info = Filght_Normal_Info;
		break;

	case EnemyType::FLIGHT_DASHER: 
		m_Info = Filght_Dasher_Info; 
		break;
	}

	m_HP = m_Info.MaxHP;

	if (m_pModel)
	{
		m_BottomOffset = fabs(m_pModel->Local_AABB.Min.y) * m_Info.Scale;
	}
}

void Enemy::Deactivate()
{
	if (!m_IsActive) return;

	m_IsActive = false;

	if (Game_Check_Is_Resetting())
	{
		return;
	}

	Audio_Manager::GetInstance()->Play_SFX("Enemy_Dead");

	float effectScale = m_Info.Scale * 0.6f;
	Billboard_Manager::Instance().Create_Effect(Position, 0, effectScale, Effect_Type::EXPLOSION);
}

void Enemy::Update(double elapsed_time)
{
	float dt = static_cast<float>(elapsed_time);
}

void Enemy::Draw()
{
	if (!m_pModel) return;

	XMFLOAT4 Color = { 1.0f, 1.0f, 1.0f, 1.0f }; // Normal >> White

	switch (m_Info.Type)
	{
	case EnemyType::GROUND_NORMAL:
		Color = { 0.2f, 1.0f, 0.2f, 1.0f }; // Green
		break;
	case EnemyType::GROUND_DASHER:
		Color = { 1.0f, 0.2f, 0.2f, 1.0f }; // Red
		break;
	case EnemyType::GROUND_TANKER:
		Color = { 0.2f, 0.2f, 1.0f, 1.0f }; // Blue
		break;
	case EnemyType::FLIGHT_NORMAL:
		Color = { 1.0f, 1.0f, 0.0f, 1.0f }; // Yellow
		break;
	case EnemyType::FLIGHT_DASHER:
		Color = { 1.0f, 0.5f, 0.0f, 1.0f }; // Orange
		break;
	}

	Shader_Manager::GetInstance()->SetDiffuseColor(Color);

	XMMATRIX mtxScale = XMMatrixScaling(m_Info.Scale, m_Info.Scale, m_Info.Scale);
	XMMATRIX mtxRot = XMMatrixRotationY(Rotation.y);
	XMMATRIX mtxTrans = XMMatrixTranslation(Position.x, Position.y, Position.z);
	XMMATRIX mtxWorld = mtxScale * mtxRot * mtxTrans;

	ModelDraw(m_pModel, mtxWorld);

	Shader_Manager::GetInstance()->SetDiffuseColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	if (Get_Debug_Mode_State())
	{
		Debug_Collision_Draw(GetAABB(), { 1.0f, 0.0f, 0.0f, 1.0f });
	}
}

void Enemy::Draw_Shadow(const DirectX::XMMATRIX& LightViewProj)
{
	if (!m_IsActive || !m_pModel) return;

	// Set World Matrix (S > R > T)
	XMMATRIX mtxScale = XMMatrixScaling(m_Info.Scale, m_Info.Scale, m_Info.Scale);
	XMMATRIX mtxRot = XMMatrixRotationY(Rotation.y);
	XMMATRIX mtxTrans = XMMatrixTranslation(Position.x, Position.y, Position.z);

	XMMATRIX mtxWorld = mtxScale * mtxRot * mtxTrans;

	// Send Matrix To Shader Manager
	Shader_Manager::GetInstance()->SetShadowWorldMatrix(mtxWorld, LightViewProj);

	// Drow Model Without Texture (Just Draw Shape)
	Shader_Manager::GetInstance()->DrawModelShadow(m_pModel, mtxWorld, LightViewProj);
}

void Enemy::OnDamage(int damage)
{
	if (m_HP <= 0 || !m_IsActive) return;

	if (!m_IsActive)
	{
		return;
	}
	int FinalDamage = damage;

	if (FinalDamage < 1)
	{
		FinalDamage = 1;
	}

	m_HP -= FinalDamage;

	DirectX::XMFLOAT3 spawnPos = Position;
	spawnPos.y += m_Info.Scale * 0.8f; // Position Above Enemy Scale * 0.8f

	// Make Damage Text
	Damage_Text_Manager::GetInstance().Spawn_Damage(spawnPos, damage,
		{ 1.0f, 0.2f, 0.2f, 1.0f }, // Red
		1.5f                        // Scale
	);

	if (m_HP <= 0)

	{
		Drop_Reward();
		Deactivate();
	}
}

AABB Enemy::GetAABB() const
{
	if (!m_pModel) return AABB{ {0,0,0}, {0,0,0} };

	AABB worldAABB = {};

	XMVECTOR Min = XMLoadFloat3(&m_pModel->Local_AABB.Min) * m_Info.Scale;
	XMVECTOR Max = XMLoadFloat3(&m_pModel->Local_AABB.Max) * m_Info.Scale;
	XMVECTOR Pos = XMLoadFloat3(&Position);

	XMStoreFloat3(&worldAABB.Min, Min + Pos);
	XMStoreFloat3(&worldAABB.Max, Max + Pos);

	return worldAABB;
}

void Enemy::Drop_Reward()
{
	Drop_Exp_And_HP();
	Drop_Weapon();
}

void Enemy::Drop_Exp_And_HP() const
{
	if (m_Info.Type == EnemyType::GROUND_TANKER)
	{
		Resource_Manager::GetInstance().Spawn_Resource(Position, Resource_Type::HEALTH_POT, 25.0f);
	}
	Resource_Manager::GetInstance().Spawn_Resource(Position, Resource_Type::EXP_ORB, m_Info.EXP_Mount);
}

void Enemy::Drop_Weapon()
{
	// Check Inventory
	const auto& Inventory = Weapon_System::GetInstance().GetInventory();
	size_t InvenSize = Inventory.size();

	// Get Enemy Type
	bool Is_Special = (m_Info.Type == EnemyType::GROUND_TANKER);
	// If, Expend Special Type, Inpt Here

	// If Drop Failed, Return
	if (!Is_Weapon_Drop(InvenSize, Is_Special))
	{
		return;
	}

	// Check Weapon Lock State From Card
	int Locked_ID = Upgrade_System::GetInstance().Get_Weapon_Lock_State();

	// Set Weapon Type
	WeaponType Drop_Type = Get_Weapon_Probabilities(Locked_ID, Is_Special, Inventory);

	// Make Weapon Box
	Resource_Manager::GetInstance().Spawn_Resource(Position, Resource_Type::WEAPON_BOX, 0.0f, Drop_Type);
}

bool Enemy::Is_Weapon_Drop(size_t InvenSize, bool Is_Special)
{
	// --------------------------------------------------------
	//                  Weapon Drop Logic
	// --------------------------------------------------------
	
	// If Inventory Is Full, Do Not Drop
	if (InvenSize >= 3)
	{
		return false; // Full
	}

	// Get Drop Chance
	float Drop_Chance = 0.0f;

	if (Is_Special)
	{
		Drop_Chance = 1.0f; // InvenSize 1, 2 >> 100%
	}
	else
	{
		if (InvenSize <= 1) 
		{
			Drop_Chance = 1.0f; // InvenSize 1 >> 100%
		}
		else                
		{
			Drop_Chance = 0.5f; // InvenSize 2 >> 50%
		}
	}

	// Dice Roll
	if (RandomFloat() > Drop_Chance)
	{
		return false; // Fail
	}

	return true;
}

WeaponType Enemy::Get_Weapon_Probabilities(int Locked_ID, bool Is_Special, const std::deque<WeaponState>& Inventory)
{
	// --------------------------------------------------------
	//               Weapon Type Selection Logic
	// --------------------------------------------------------

	// Set Weapon Type
	WeaponType W_Type = WeaponType::HANDGUN;

	// If Weapon Type Locked
	if (Locked_ID != -1)
	{
		W_Type = static_cast<WeaponType>(Locked_ID); // Only Drop Locked Weapon Type
	}
	else
	{
		// Get Weapon Probabilities
		float Probs[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		// Set Default Probabilities
		if (Is_Special)
		{
			// Special Type
			Probs[0] = 0.25f; Probs[1] = 0.25f; Probs[2] = 0.25f; Probs[3] = 0.25f;
		}
		else
		{
			// Normal Type
			Probs[0] = 0.30f; Probs[1] = 0.30f; Probs[2] = 0.25f; Probs[3] = 0.15f;
		}

		// Check Same Weapon Count
		int Counts[4] = { 0, 0, 0, 0 };
		for (const auto& w : Inventory)
		{
			Counts[static_cast<int>(w.Spec.Type)]++;
		}

		// Set Probabilities
		for (int i = 0; i < 4; ++i)
		{
			if (Counts[i] >= 2)
			{
				// If Same Weaopn Count Over 2, Probabilities Decrease 0.15
				float penalty = 0.15f;
				if (Probs[i] < penalty) 
				{
					penalty = Probs[i]; // Safety Code
				}

				Probs[i] -= penalty;

				// Other Weapon Probabilities Increase 0.05
				for (int j = 0; j < 4; ++j)
				{
					if (i != j) Probs[j] += 0.05f;
				}
			}
		}

		// Get Weighted Random
		float Dice = RandomFloat(); // 0.0 ~ 1.0
		float Cumulative = 0.0f;

		for (int i = 0; i < 4; ++i)
		{
			// Check Probabilities
			Cumulative += Probs[i];

			if (Dice <= Cumulative)
			{
				W_Type = static_cast<WeaponType>(i);
				break;
			}
		}
	}

	return W_Type;
}

void Enemy::Enemy_Collision_Map(double elapsed_time)
{
	float dt = static_cast<float>(elapsed_time);

	AABB E_AABB = GetAABB();
	int OBJ_Count = Map_System_Get_Object_Count();

	XMVECTOR Enemy_POS = XMLoadFloat3(&Position);

	for (int i = 0; i < OBJ_Count; i++)
	{
		const MapObject* OBJ = Map_System_Get_Object(i);

		if (OBJ->OBJ_ID == FIELD) continue;

		IsHit hit = Collision_Is_Hit_AABB(E_AABB, OBJ->Collision);

		if (hit.Is_Hit)
		{
			XMVECTOR vNormal = XMLoadFloat3(&hit.Normal);
			XMVECTOR vPush = vNormal * hit.Depth;

			Enemy_POS += vPush;

			XMStoreFloat3(&Position, Enemy_POS);
			E_AABB = GetAABB();
		}
	}
}

void Enemy::Enemy_Collision_Player(double elapsed_time)
{
	float dt = static_cast<float>(elapsed_time);

	XMFLOAT3 P_Pos = Player_Get_POS();
	XMFLOAT3 E_Pos = Position;

	// 1. Set Y Axis And Lock
	XMVECTOR V_P_Pos = XMLoadFloat3(&P_Pos);
	XMVECTOR V_E_Pos = XMLoadFloat3(&E_Pos);

	V_P_Pos = XMVectorSetY(V_P_Pos, 0.0f);
	V_E_Pos = XMVectorSetY(V_E_Pos, 0.0f);

	// 2. Get Dist
	XMVECTOR V_Dist = XMVector3Length(V_E_Pos - V_P_Pos);
	float dist = XMVectorGetX(V_Dist);

	// 3. Set AABB Range
	AABB P_AABB = Player_Get_AABB();
	float P_Width = P_AABB.Max.x - P_AABB.Min.x;
	float P_Radius = P_Width * 0.5f;

	float E_Radius = m_Info.Scale * 0.5f;

	// Get Real Core
	float Collision_Radius = (P_Radius + E_Radius) * 0.5f;

	// 4. Collision Check And Push Back
	if (dist < Collision_Radius)
	{
		if (dist < 0.001f)
		{
			Position.x += 0.1f;
			return;
		}

		Player_OnDamage(m_Info.Collision_Damage);

		XMVECTOR V_Dir = V_E_Pos - V_P_Pos;
		V_Dir = XMVector3Normalize(V_Dir);

		float pushDist = Collision_Radius - dist;
		XMVECTOR V_Current_Pos = XMLoadFloat3(&Position);
		V_Current_Pos += V_Dir * pushDist;

		XMStoreFloat3(&Position, V_Current_Pos);
	}
}

void Enemy::Enemy_Collision_Enemy(double elapsed_time)
{
	float dt = static_cast<float>(elapsed_time);

	std::vector<Enemy*>& activeList = Enemy_Manager::GetInstance().Get_Active_List();

	AABB My_AABB = GetAABB();
	XMVECTOR My_Pos = XMLoadFloat3(&Position);

	for (const auto& other : activeList)
	{
		// Skip Self, Or Dead Enemy
		if (!other->IsActive() || other->IsDead())
		{
			continue;
		}

		IsHit Hit = Collision_Is_Hit_AABB(My_AABB, other->GetAABB());

		if (Hit.Is_Hit)
		{
			XMVECTOR vNormal = XMLoadFloat3(&Hit.Normal);
			XMVECTOR vDepth = XMLoadFloat(&Hit.Depth);

			// Each Enemy Push Out Half Distance
			My_Pos += vNormal * vDepth * 0.5f;
		}
	}
	XMStoreFloat3(&Position, My_Pos);
}