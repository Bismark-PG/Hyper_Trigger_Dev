/*==============================================================================

	Manage Player Logic [Player.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef PLAYER_H
#define PLAYER_H
#include <DirectXMath.h>
#include "Collision.h"

enum class E_PlayerState
{
	IDLE,
	WALK,
	JUMP
};

void Player_Initialize(const DirectX::XMFLOAT3& First_POS, const DirectX::XMFLOAT3& First_Front);
void Player_Finalize();

void Player_Update(double elapsed_time);
void Player_Model_Animation_Update(float Time);

void Player_Draw();
void Player_Draw_Shadow(const DirectX::XMMATRIX& LightViewProj);

void Player_Reset();
void Player_Set_Sprint_Toggle_Mode(bool isToggle);

void Player_Set_POS(DirectX::XMFLOAT3& POS);
const DirectX::XMFLOAT3& Player_Get_POS();
const DirectX::XMFLOAT3& Player_Get_Front();

AABB Player_Get_AABB();

void Player_OnDamage(int damage);
void Player_Heal(int amount);
int Player_Get_HP();
int Player_Get_MaxHP();

bool Player_Check_Is_Dead();
bool Player_Is_Aiming_Now();

void Player_LevelUp();
float Player_Get_Damage_Coefficient();

void Player_Add_Damage_Bonus(float amount);
void Player_Add_Speed_Bonus(float amount);
void Player_Add_HP_Bonus(float amount);

// --- Parameter Geter  ---
bool Is_Player_Move();
bool Is_Player_Jump();
bool Is_Player_Shoot();
bool Is_Plyer_Run();

#endif // PLAYER_H