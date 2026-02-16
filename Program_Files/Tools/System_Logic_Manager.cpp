/*==============================================================================

	Manage System Logic [System_Logic_Manager.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "System_Logic_Manager.h"
#include <Debug_Collision.h>

void System_Manager::Initialize(HWND hWnd, ID3D11Device* Device, ID3D11DeviceContext* Context)
{
	// Initialize System Tools
	SystemTimer_Initialize();
	KeyLogger_Initialize(hWnd);

	// Initialize Audio Tools
	Audio_Manager::GetInstance()->Init();
	Game_Audio_Initialize();

	// Initialize Shader Tools
	Shader_Manager::GetInstance()->Init(Device, Context);
	Shader_Manager::GetInstance()->InitShadow();
	Light_Manager::GetInstance().Init();

	// Initialize Texture Tools
	Texture_Manager::GetInstance()->Init(Device, Context);
	Game_Texture_Initialize();
	Model_Manager::GetInstance()->Init();
	Sprite_Initialize(Device, Context);
	SpriteAni_Initialize();
	Billboard_Initialize();
	Billboard_Manager::Instance().Init();

	// Initialize Game Setting Tools
	Fade_Initialize();
	Mouse_UI_set();
	Main_Logic_Initialize();
	Game_Initialize(Device, Context);

	// Initialize Debug Tools
	GUI_Init(hWnd, Device, Context);
	Debug_Camera_Initialize();
	Debug_Collision_Initialize(Device);
}

void System_Manager::Finalize()
{
	// Finalize Debug Tools
	Debug_Collision_Finalize();
	Debug_Camera_Finalize();
	GUI_Final();

	// Finalize Game Setting Tools
	Game_Finalize();
	Main_Logic_Finalize();
	Fade_Finalize();

	// Finalize Texture Tools
	Billboard_Finalize();
	Billboard_Manager::Instance().Final();
	SpriteAni_Finalize();
	Sprite_Finalize();
	Model_Manager::GetInstance()->Final();
	Game_Texture_Finalize();
	Texture_Manager::GetInstance()->Final();

	// Finalize Shader Tools
	Shader_Manager::GetInstance()->FinalShadow();
	Shader_Manager::GetInstance()->Final();

	// Finalize Audio Tools
	Game_Audio_Finalize();
	Audio_Manager::GetInstance()->Final();

	// Finalize System Tools
	Direct3D_Finalize();
	Mouse_Finalize();
}