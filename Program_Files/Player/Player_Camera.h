/*==============================================================================

	Make Camera [Player_Camera.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef PLAYER_CAMERA_H
#define PLAYER_CAMERA_H

#include <DirectXMath.h>

enum class Player_Sights
{
	Left,
	Middle,
	Right
};

void Player_Camera_Initialize();
void Player_Camera_Finalize();

void Player_Camera_Reset();
void Player_Camera_Update(double elapsed_time);

void Player_Camera_Set_Menu_Mode(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target);
void Player_Camera_Set_Game_Mode();

const DirectX::XMFLOAT3& Player_Camera_Get_POS();
const DirectX::XMFLOAT3& Player_Camera_Get_Front();

void Set_Mouse_Sensitivity(float Sensitivity);
float Get_Mouse_Sensitivity();

const DirectX::XMFLOAT3& Player_Camera_Get_Current_POS();
float Player_Camera_Get_Yaw();
float Player_Camera_Get_Pitch();
float Player_Camera_Get_Far_Z();

Player_Sights Player_Camera_Get_Now_Sights();
void Player_Camera_Set_Sights(Player_Sights sight);

void Player_Camera_Set_Aiming_Mode(bool Is_Aiming);

DirectX::XMFLOAT4X4& Player_Camera_Get_View_Matrix();

#endif //PLAYER_CAMERA_H
