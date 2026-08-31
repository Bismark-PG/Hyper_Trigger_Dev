/*==============================================================================

	Project Main CPP [Update_Game.cpp]

	Project : DirectX 3D Sample

	Author : Choi HyungJoon
	Date : 2026. 03 ~
	Version : 1.1.0
	Note : Deving Now......

	Version List :
		Date : 2026. 01. 13
		Version : Prototype
		Note : Prototype Completed

		Date : 2026. 02. 13
		Version : 1.0.0
		Note : Initial Release!

==============================================================================*/
#define WIN32_LEAN_AND_MEAN
#include "System_Logic_Manager.h"
#include "Game_Header_Manager.h"
using namespace PALETTE;
using namespace DirectX;

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR IpCmdline, _In_ int nCmdShow)
{
	// Do Not Show Error For Not Used Resource, Not Necessary
	UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(IpCmdline);

	// Make Game Loop & Message Loop
	MSG Message = {};

	// Initialize COM Library
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Failed To Set Function In Main.cpp : [CoInitializeEx]", L">> FATAL ERROR <<", MB_OK | MB_ICONERROR);
		return static_cast<int>(Message.wParam);
	}

	// Set DPI Aware
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// Create Window
	HWND hWnd = Window_Manager::GetInstance()->Init(hInstance, nCmdShow);
	if (!hWnd)
	{
		MessageBoxW(nullptr, L"Failed To Set Window : [HWND]", L">> FATAL ERROR <<", MB_OK | MB_ICONERROR);
		return static_cast<int>(Message.wParam);
	}

	// Initialize Direct 3D
	Direct3D_Initialize(hWnd);
	ID3D11Device* Device = Direct3D_GetDevice();
	if (!Device)
	{
		MessageBoxW(nullptr, L"Failed To Set DirectX System : [Device]", L">> FATAL ERROR <<", MB_OK | MB_ICONERROR);
		return static_cast<int>(Message.wParam);
	}
	
	ID3D11DeviceContext* Context = Direct3D_GetContext();
	if (!Context)
	{
		MessageBoxW(nullptr, L"Failed To Set DirectX System : [Context]", L">> FATAL ERROR <<", MB_OK | MB_ICONERROR);
		return static_cast<int>(Message.wParam);
	}

	// Initialize System Logic
	System_Manager::GetInstance().Initialize(hWnd, Device, Context);

	// Create Debug Text
	std::unique_ptr<Text::DebugText> Debug;
	auto Create_DebugText = [&]()
		{
			Debug = std::make_unique<Text::DebugText>(
				Device, Context,
				Texture_Manager::GetInstance()->GetID("Debug_Text"),
				Direct3D_GetBackBufferWidth(), Direct3D_GetBackBufferHeight());
			Debug->SetScale(0.75f);
			Debug->SetOffset(10.0f, 10.0f);
		};
	Create_DebugText();

	// Show Mouse (True = Show // False = Don`t Show)
	// Mouse_SetVisible(false);
	// Mouse Postion Mode
	// Mouse_SetMode(MOUSE_POSITION_MODE_RELATIVE);

	// Show Window
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Set Frame
	double Exec_Last_Time = SystemTimer_GetTime();
	double FPS_Last_Time = Exec_Last_Time;
	double Current_Time = 0.0;
	ULONG Frame_Count = 0;
	double FPS = 0.0; // Frame Per Second
	bool IS_Controller_Set = false;

	do
	{
		if (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		
		// Set Game
		else
		{
			if (IF_IS_Game_Done())
			{
				// Game Done (EXIT)
				if (Fade_GetState() == FADE_STATE::FINISHED_OUT)
				{
					PostQuitMessage(0);
				}
			}

			// Time Set
			Current_Time = SystemTimer_GetTime(); // Get System Time
			double Elapsed_Time = Current_Time - FPS_Last_Time; // Get Time For FPS

			//  Set FPS
			if (Elapsed_Time >= 1.0)
			{
				FPS = Frame_Count / Elapsed_Time;
				FPS_Last_Time = Current_Time; // Save FPS
				Frame_Count = 0; // Clear Count
			}

			// Set Elapsed Time
			Elapsed_Time = Current_Time - Exec_Last_Time;
			// If you want to limit FPS, set condition here
			if (Elapsed_Time >= (1.0 / 60.0)) // 60 FPS
			{
				Exec_Last_Time = Current_Time; // Save Last Time
				
				Fade_Update(Elapsed_Time);

				// Set Key Logger With FPS
				KeyLogger_Update();
				Controller_Set_Update();
				IS_Controller_Set = Controller_Set_UP();

				// Update Game Texture
				if (IS_Controller_Set)
				{
					SpriteAni_Update(Elapsed_Time);
				}
				else if (!IS_Controller_Set)
				{
					Main_Logic_Update(Elapsed_Time);
					SpriteAni_Update(Elapsed_Time);
				}

				// Draw Texture
				Direct3D_Clear();
				Sprite_Begin();

				// Real Draw Startq
				Main_Game_Screen_Update();

				// Controller Input Alert
				Controller_Set_Draw();

				// Draw GUI
				if (Get_Debug_Mode_State())
				{
					// Start the Dear ImGui frame
					ImGui_ImplDX11_NewFrame();
					ImGui_ImplWin32_NewFrame();
					ImGui::NewFrame();
					ImGui::GetIO().MouseDrawCursor = true;
					
					// Player, Camera, Debug Collision, Animation Model
					GUI_Model_Editor(FPS);

					// Light, Map System
					GUI_World_Editor();

					ImGui::Render();
					ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
				}

#if defined(DEBUG) || defined(_DEBUG)
				std::stringstream Debug_FPS_State;
				Debug_FPS_State << "FPS : " << FPS << std::endl;
				Debug_FPS_State << "Main : " << static_cast<int>(Game_Manager::GetInstance()->Get_Current_Main_Screen()) << std::endl;
				Debug_FPS_State << "Sub  : " << static_cast<int>(Game_Manager::GetInstance()->Get_Current_Sub_Screen()) << std::endl;
				Debug_FPS_State << "Game : " << static_cast<int>(Game_Manager::GetInstance()->Get_Current_Game_Select_Screen()) << "\n" << std::endl;
				Debug_FPS_State << "Main Buffer : " << static_cast<int>(Get_Main_Menu_Buffer()) << std::endl;
				Debug_FPS_State << "Setting Buffer : " << static_cast<int>(Get_Setting_Buffer()) << std::endl;
				Debug_FPS_State << "Setting State : " << static_cast<int>(Get_Setting_State()) << std::endl;
				Debug_FPS_State << "Select State : " << static_cast<int>(Get_Select_Menu_Buffer()) << std::endl;
				Debug_FPS_State << "Game Mode : " << static_cast<int>(Get_Game_Mode()) << "\n" << std::endl;
				Debug_FPS_State << "BGM : " << static_cast<int>(Get_BGM_Scale_Buffer()) << std::endl;
				Debug_FPS_State << "SFX : " << static_cast<int>(Get_SFX_Scale_Buffer()) << std::endl;

				Debug->Print(Debug_FPS_State.str().c_str(), Light_Green);

				Debug->Draw();
				Debug->Clear();
#endif	

				// Fade Draw
				Fade_Draw();

				Direct3D_Present();

				Frame_Count++;
			}	
		}
	} while (Message.message != WM_QUIT);
	
	System_Manager::GetInstance().Finalize();

	return static_cast<int>(Message.wParam);
}