/*==============================================================================

	Draw Sky Model [Sky.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "Sky.h"
#include "Game_Model.h"
#include "Shader_Manager.h"
#include "Player_Camera.h"
#include <Player.h>
#include <Light_Manager.h>
#include <direct3d.h>

using namespace DirectX;

static MODEL* g_pModelSky = nullptr;
static DirectX::XMFLOAT3 g_SkyPos = { 0.0f, 0.0f, 0.0f };

void Sky_Initialize()
{
    g_pModelSky = Model_Manager::GetInstance()->GetModel("Sky");
}

void Sky_Finalize()
{
    g_pModelSky = nullptr;
}

void Sky_Update()
{
    g_SkyPos = Player_Get_POS();
}

void Sky_Draw()
{
    if (!g_pModelSky) return;

	// Unlighting for Skybox
    // Shader_Manager::GetInstance()->SetLightAmbient({ 1.0f, 1.0f, 1.0f, 1.0f });
    // Shader_Manager::GetInstance()->SetLightDirectional({ 0, -1, 0, 0 }, { 0, 0, 0, 0 });

	// World Matrix
    XMMATRIX mtxScale = XMMatrixScaling(50.0f, 50.0f, 50.0f);
    XMMATRIX mtxRot   = XMMatrixRotationX(XMConvertToRadians(90.0f));
    XMMATRIX mtxTrans = XMMatrixTranslation(g_SkyPos.x, g_SkyPos.y, g_SkyPos.z);
    XMMATRIX mtxWorld = mtxScale * mtxRot * mtxTrans;

	// Draw Sky Model
    Direct3D_SetDepthEnable(false);
    ModelDraw(g_pModelSky, mtxWorld);
    Direct3D_SetDepthEnable(true);
}