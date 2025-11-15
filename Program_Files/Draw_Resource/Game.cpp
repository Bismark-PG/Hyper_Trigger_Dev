/*==============================================================================

	Game [Game.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "Game.h"
#include "Effect.h"
#include "Cube.h"
#include "Grid.h"
#include "direct3d.h"
#include "KeyLogger.h"
#include <DirectXMath.h>
#include "Mash_Field.h"
#include "Player.h"
#include "Player_Camera.h"
#include "Palette.h"
#include "Model.h"
#include "Game_Model.h"
#include "Bullet.h"
#include "Debug_Collision.h"
#include "Game_UI.h"
#include "Billboard.h"
#include "Debug_Camera.h"
using namespace DirectX;
using namespace PALETTE;

static XMFLOAT3 Player_First_POS = { 0.0f, 5.0f, -5.0f };

static MODEL* CarModel = nullptr;
static MODEL* BallModel = nullptr;

static bool Is_Debug_Camera = false;
static bool Is_Sights_Change = false;

void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Player_Initialize(Player_First_POS, { 0.0f, 0.0f, 1.0f });
	Player_Camera_Initialize();

	Grid_Initialize(pDevice, pContext);
	Cube_Initialize(pDevice, pContext);
	Mash_Field_Initialize(pDevice, pContext);
	Billboard_Initialize();

	Debug_Camera_Initialize();
	Debug_Collision_Initialize(pDevice);
	Game_UI_Initialize();

	BallModel = Model_M->GetModel("ball");
	CarModel = Model_M->GetModel("car");
}

void Game_Finalize()
{
	Game_UI_Finalize();
	Debug_Collision_Finalize();
	Debug_Camera_Finalize();

	Billboard_Finalize();
	Mash_Field_Finalize();
	Cube_Finalize();
	Grid_Finalize();

	Player_Camera_Finalize();
	Player_Finalize();
}

void Game_Update(double elapsed_time)
{
	if (KeyLogger_IsTrigger(KK_V) && !Is_Sights_Change)
	{
		Is_Sights_Change = true;
		Player_Camera_Set_Now_Sights();
		Is_Sights_Change = false;
	}

	if (KeyLogger_IsTrigger(KK_F1))
	{
		Is_Debug_Camera = !Is_Debug_Camera;
		Debug_Camera_Set_Position(Player_Camera_Get_Current_POS());
		Debug_Camera_Set_Rotation(Player_Camera_Get_Yaw(), Player_Camera_Get_Pitch());
	}

	if (Is_Debug_Camera)
	{
		Debug_Camera_Update(elapsed_time);
	}
	else
	{
		Player_Update(elapsed_time);
		Bullet_Update(elapsed_time);
		Player_Camera_Update(elapsed_time);
	}

	// --- Set Light ---
	Shader_M->SetLightAmbient({ 0.1f, 0.1f, 0.1f, 1.0f });
	Shader_M->SetLightDirectional({ 0.0f, -1.0f, 0.0f, 0.0f }, { 0.8f, 0.8f, 0.8f, 1.0f });
	Shader_M->SetDiffuseColor(White);

	// --- Point Light ---
	Shader_M->SetPointLightCount(4);

	XMFLOAT3 playerPos = Player_Get_POS();
	playerPos.y += 1.0f;
	Shader_M->SetPointLight(0, playerPos, 3.0f, { 1.0f, 0.5f, 0.0f, 1.0f });
	Shader_M->SetPointLight(1, { 0.0f, 1.0f, 0.0f }, 1.0f, { 1.0f, 0.0f, 0.0f, 5.0f });
	Shader_M->SetPointLight(2, { 1.5f, 1.0f, 0.0f }, 1.0f, { 0.0f, 1.0f, 0.0f, 5.0f });
	Shader_M->SetPointLight(3, { 3.0f, 1.0f, 0.0f }, 1.0f, { 0.0f, 0.0f, 1.0f, 5.0f });
}

void Game_Draw()
{
	Direct3D_SetDepthEnable(true);

	Mash_Field_Draw();
	Player_Draw();
	Bullet_Draw();
	Billboard_Draw(1, { -5.0f, 0.5f, -5.0f}, 1.0f, 1.0f);

	if (CarModel)
	{
		XMMATRIX mtxRot = XMMatrixRotationZ(XMConvertToRadians(180));
		XMMATRIX mtxCar_move = XMMatrixTranslation(-3.0f, 0.0f, 3.0f);
		XMMATRIX mtxCar = mtxRot * mtxCar_move;
		ModelDraw(CarModel, mtxCar);
	}

	XMMATRIX mtxWorld = XMMatrixIdentity();
	Cube_Draw(mtxWorld, Shader_Filter::MAG_MIP_POINT);
	mtxWorld = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	ModelDraw(BallModel, mtxWorld);

	mtxWorld = XMMatrixTranslation(1.5f, 0.0f, 0.0f);
	Cube_Draw(mtxWorld, Shader_Filter::MAG_MIP_LINEAR);
	mtxWorld = XMMatrixTranslation(1.5f, 1.0f, 0.0f);
	ModelDraw(BallModel, mtxWorld);

	mtxWorld = XMMatrixTranslation(3.0f, 0.0f, 0.0f);
	Cube_Draw(mtxWorld, Shader_Filter::ANISOTROPIC);
	mtxWorld = XMMatrixTranslation(3.0f, 1.0f, 0.0f);
	ModelDraw(BallModel, mtxWorld);

	// Draw UI
	if (!Is_Debug_Camera)
	{
		Direct3D_SetDepthEnable(false);
		Shader_M->Begin2D();
		Game_UI_Draw();
	}
}

// Make pyramid
//int d = 1, i = 5;
//for (int j = 0; j < 2; j++)
//{
//	for (int k = 0; k < 2; k++)
//	{
//		XMMATRIX mtxOffsec = XMMatrixTranslation(-2.5f + i * j, 0.0f, 2.5f - i * k);
//		
//		for (int y = 0; y < 5 * d; y++)
//		{
//			for (int z = 0; z < 5 * d - y; z++)
//			{
//				for (int x = 0; x < 5 * d - y; x++)
//				{
//					XMMATRIX mtxTrans = XMMatrixTranslation(-2.0f * d + x + 0.5f * y, 0.5f + y, -2.0f * d + z + 0.5f * y);
//					XMMATRIX mtxRotateY = XMMatrixRotationY(XMConvertToRadians(g_Angle));
//					XMMATRIX mtxScale = XMMatrixScaling(1.0f, g_Scale, 1.0f);
//					
//					XMMATRIX mtxWorld = mtxTrans * mtxRotateY * mtxOffsec * mtxScale;
//					
//					Cube_Draw(mtxWorld);
//				}
//			}
//		}
//	}
//}