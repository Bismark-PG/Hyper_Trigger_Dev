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
#include "Game_UI.h"
#include "Billboard.h"
#include "Debug_Camera.h"
#include "Map_System.h"
#include "Sky.h"
#include "Enemy_Manager.h"
#include "Light_Manager.h"
#include "Texture_Manager.h"
#include "Billboard_Manager.h"
#include "Bullet_Manager.h"
#include "Resource_Manager.h"
#include "Particle_Manager.h"
#include "Enemy_Spawner.h"
#include "In_Game_Menu.h"
#include "Game_Screen_Manager.h"
#include "Audio_Manager.h"
#include "BGM_Mixer.h"
#include "Wave_Data.h"
#include "Fade.h"
#include "Weapon_System.h"
#include "Upgrade_System.h"
using namespace DirectX;
using namespace PALETTE;

static XMFLOAT3 Player_First_POS = { 0.0f, 5.0f, -5.0f };

static bool Is_Debug_Mode = false;

static float Game_Play_Time = 0.0f;
static int Last_Processed_Loop = -1;
static bool Is_Game_Clear_Sequence = false;
static bool Is_Music_Synced = false;
static bool Is_Reset_Mode = false;

void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Player_Initialize(Player_First_POS, { 0.0f, 0.0f, 1.0f });
	Player_Camera_Initialize();

	Grid_Initialize(pDevice, pContext);
	Cube_Initialize(pDevice, pContext);
	Mash_Field_Initialize(pDevice, pContext);
	Map_System_Initialize();

	Sky_Initialize();
	Enemy_Manager::GetInstance().Init();
	Enemy_Spawner::GetInstance().Init();

	Particle_Manager::GetInstance().Init();
	Resource_Manager::GetInstance().Init();
	Upgrade_System::GetInstance().Init();

	Game_UI_Initialize();
}

void Game_Finalize()
{
	Game_UI_Finalize();
	
	Upgrade_System::GetInstance().Final();
	Resource_Manager::GetInstance().Final();
	Particle_Manager::GetInstance().Final();

	Enemy_Manager::GetInstance().Final();
	Sky_Finalize();

	Map_System_Finalize();
	Mash_Field_Finalize();
	Cube_Finalize();
	Grid_Finalize();

	Player_Camera_Finalize();
	Player_Finalize();
}

void Game_Update(double elapsed_time)
{
	float dt = static_cast<float>(elapsed_time);

	if (KeyLogger_IsTrigger(KK_F1))
	{
		Is_Debug_Mode = !Is_Debug_Mode;

		Debug_Mode_Switcher();
		Debug_Mode_Set();

		if (Is_Debug_Mode)
		{
			Debug_Camera_Set_Position(Player_Camera_Get_Current_POS());
			Debug_Camera_Set_Rotation(Player_Camera_Get_Yaw(), Player_Camera_Get_Pitch());
		}
		else 
		{
		    Mouse_SetMode(MOUSE_POSITION_MODE_RELATIVE);
		}
	}

	if (Upgrade_System::GetInstance().Is_Active())
	{
		Upgrade_System::GetInstance().Update();
		return;
	}

	if (Get_Debug_Mode_State())
	{
		Debug_Camera_Update(elapsed_time);
		Player_Model_Animation_Update(dt);
		Map_System_Update(elapsed_time);
		return;
	}

	if (Is_Game_Clear_Sequence)
	{
		float Current_Vol = Audio_Manager::GetInstance()->Get_Target_BGM_Volume();

		float Fade_Out_Speed = 1.0f / 3.0f;

		if (Current_Vol > 0.0f)
		{
			Current_Vol -= Fade_Out_Speed * dt;
			if (Current_Vol < 0.0f) Current_Vol = 0.0f;

			Audio_Manager::GetInstance()->Set_Target_BGM_Volume(Current_Vol);
		}

		if (Fade_GetState() == FADE_STATE::FINISHED_OUT)
		{
			Mixer_Init();

			Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);

			Game_Manager::GetInstance()->Update_Main_Screen(Main_Screen::MENU_SELECT);
			Game_Manager::GetInstance()->Update_Game_Select_Screen(Game_Select_Screen::G_WAIT);

			Fade_Start(1.5f, false);
		}
		return;
	}

	Mixer_First_Game_Start(elapsed_time);

	if (!Is_Music_Synced)
	{
		if (Audio_Manager::GetInstance()->Get_Target_BGM_Volume() > 0.01f)
		{
			Is_Music_Synced = true;
			Weapon_System::GetInstance().SyncBGM();
			Debug::D_Out << "[DJ System] Sync Started via Volume/Loop Check" << std::endl;
		}
	}

	if (Is_Music_Synced)
	{
		int Current_Loop = Audio_Manager::GetInstance()->Get_Current_Loop_Count();

		if (Current_Loop > Last_Processed_Loop)
		{
			Weapon_System::GetInstance().SyncBGM();

			Debug::D_Out << "[DJ System] Loop " << Current_Loop << " : Weapons Synced" << std::endl;

			Last_Processed_Loop = Current_Loop;
		}
	}

	if (KeyLogger_IsTrigger(KK_ESCAPE) || KeyLogger_IsTrigger(KK_BACK) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_START))
	{
		// Audio_Manager::GetInstance()->Pause_BGM(true);
		Audio_Manager::GetInstance()->Play_SFX("Buffer_Denied");
		In_Game_Menu_Reset();
		Game_Manager::GetInstance()->Update_Game_Select_Screen(Game_Select_Screen::GAME_IN_GAME_MENU);
		return;
	}

	Map_System_Update(elapsed_time);
	Player_Update(elapsed_time);
	Bullet_Manager::Instance().Update(elapsed_time);
	Player_Camera_Update(elapsed_time);

	Sky_Update();
	Enemy_Manager::GetInstance().Update(elapsed_time);

	if (Is_Music_Synced)
	{
		Enemy_Spawner::GetInstance().Update(elapsed_time);
	}

	Particle_Manager::GetInstance().Update(elapsed_time);
	Resource_Manager::GetInstance().Update(elapsed_time);
	Game_UI_Update(elapsed_time);
}

void Game_Draw()
{
	// =========================================================
	//				     [ Draw Shadow Pass ]
	// =========================================================
 
	// Get Light Direction, Player POS
	XMFLOAT4 Light_Dir = Light_Manager::GetInstance().Get_Directional_Vector();
	XMFLOAT3 Player_POS = Player_Get_POS();

	// Get Light View-Projection For Player Shadow
	XMMATRIX lightVP = Shader_Manager::GetInstance()->GetShadowManager()->GetLightViewProjMatrix(Light_Dir, Player_POS);

	// Draw Shadow Animation OBJ
	Shader_Manager::GetInstance()->BeginShadow_Skinning();
	Player_Draw_Shadow(lightVP);

	// Draw Shadow Map System
	Shader_Manager::GetInstance()->BeginShadow_Static();
	Map_System_Draw_Shadow(lightVP);
	Enemy_Manager::GetInstance().Draw_Shadow(lightVP);

	// End Shadow Pass
	Shader_Manager::GetInstance()->EndShadow();

	// =========================================================
	//					  [ Draw Main Pass ]
	// =========================================================

	// --- Draw Game Resources ---
	Direct3D_SetDepthEnable(true);
	Shader_Manager::GetInstance()->Begin3D();

	// Set Shader Map In Pixel Shader
	ID3D11ShaderResourceView* shadowSRV = Shader_Manager::GetInstance()->GetShadowManager()->GetShadowMapSRV();
	Shader_Manager::GetInstance()->SetShadowMapTexture(shadowSRV);

	// --- Draw Sky Model ---
	Sky_Draw();

	// --- Set Global Light ---
	Light_Manager::GetInstance().Global_Light_Set_Up();

	// --- Draw 3D Object ---
	Map_System_Draw();
	Player_Draw();
	Bullet_Manager::Instance().Draw();
	Enemy_Manager::GetInstance().Draw();

	// --- Draw 2D Object, UI ---
	Shader_Manager::GetInstance()->SetAlphaBlend(true);
	Billboard_Manager::Instance().Draw();
	Resource_Manager::GetInstance().Draw();

	// Draw UI When Not Debug Mod
	if (!Get_Debug_Mode_State())
	{
		Game_UI_Draw();
	}

	if (Upgrade_System::GetInstance().Is_Active())
	{
		Upgrade_System::GetInstance().Draw();
	}
}

bool Get_Debug_Mode_State()
{
	return Is_Debug_Mode;
}

void Set_Debug_Mode_State(bool State)
{
	Is_Debug_Mode = State;
}

void Game_Info_Reset()
{
	Is_Reset_Mode = true;

	// 1. Player Reset
	Player_Reset();
	Player_Camera_Set_Game_Mode();
	Player_Camera_Reset();

	// 2. Manager Reset
	Enemy_Manager::GetInstance().Reset();
	Bullet_Manager::Instance().Reset();
	Enemy_Spawner::GetInstance().Reset();
	Resource_Manager::GetInstance().Reset();
	Upgrade_System::GetInstance().Reset();
	Billboard_Manager::Instance().Reset();

	// 3. DJ List Reset
	Weapon_System::GetInstance().Init();

	// 4. Reset Game Time
	Game_Play_Time = 0.0f;
	Last_Processed_Loop = -1;
	Audio_Manager::GetInstance()->Reset_Loop_Count();

	Is_Game_Clear_Sequence = false;
	Is_Music_Synced = false;

	// 5. Fade
	Fade_Start(1.5f, false);

	Is_Reset_Mode = false;

	Debug::D_Out << "[Game] Info Reset Completed. Ready to Trigger!" << std::endl;
}

bool Game_Check_Is_Resetting()
{
	return Is_Reset_Mode;
}