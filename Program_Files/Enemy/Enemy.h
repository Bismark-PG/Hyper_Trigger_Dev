/*==============================================================================

	Manage Enemy [Enemy.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef ENEMY_H
#define ENEMY_H
#include <DirectXMath.h>
#include "Collision.h"
#include "model.h"
#include "Enemy_Type.h"
#include "Weapon_Info.h"

enum class EnemyState
{
	CHASE,         
	ATTACK_PREP,   
	ATTACK_ACT,    
	COOLDOWN       
};				   

class Enemy
{
public:
	Enemy(const DirectX::XMFLOAT3& pos);
	virtual ~Enemy();

	// --- Pulling System ---
	virtual void Activate(const DirectX::XMFLOAT3& pos, EnemyType type);
	virtual void Deactivate();
	bool IsActive() const { return m_IsActive; }

	// --- Main Logic ---
	virtual void Update(double elapsed_time);

	virtual void Draw();
	void Draw_Shadow(const DirectX::XMMATRIX& LightViewProj);

	void OnDamage(int damage);
	bool IsDead() const { return m_HP <= 0; }
	AABB GetAABB() const;
	DirectX::XMFLOAT3 GetPosition() const { return Position; }

protected:
	// --- Drop Resource ---
	void Drop_Reward();
	void Drop_Exp_And_HP() const;
	void Drop_Weapon();
	bool Is_Weapon_Drop(size_t InvenSize, bool Is_Special);
	WeaponType Get_Weapon_Probabilities(int Locked_ID, bool Is_Special, const std::deque<WeaponState>& Inventory);

	// --- State ---
	bool m_IsActive = false;
	Enemy_Info m_Info;

	// --- Stat ---
	int m_HP = 100;
	float m_Gravity = 0.0f;
	float m_BottomOffset = 0.0f;

	// --- Transform ---
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f }; 
	DirectX::XMFLOAT3 Rotation = { 0.0f, 0.0f, 0.0f }; 

	// --- Resource ---
	MODEL* m_pModel = nullptr;
	EnemyState m_State = EnemyState::CHASE;

	float m_StateTimer = 0.0f;
	DirectX::XMFLOAT3 m_TargetPos = {};

	// --- Physical Defense ---
	void Enemy_Collision_Map(double elapsed_time);
	void Enemy_Collision_Player(double elapsed_time);
	void Enemy_Collision_Enemy(double elapsed_time);
};
#endif // ENEMY_H