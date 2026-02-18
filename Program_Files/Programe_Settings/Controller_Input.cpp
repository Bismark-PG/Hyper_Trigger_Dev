/*==============================================================================

	Controller Input Draw [Controller_Input.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "Controller_Input.h"
#include "Audio_Manager.h"
#include "direct3d.h"
#include "Shader_Manager.h"

static int UI_Controller_BG = -1;
static int UI_Controller_Input = -1;
static int UI_Controller_Output = -1;
static int UI_Controller_Button_Up = -1;
static int UI_Controller_Button_Down = -1;
static int UI_Controller_Enter_Up = -1;
static int UI_Controller_Enter_Down = -1;

static bool Is_Pressed = false;
static double Pressed_Time = 0.0f;

static bool Controller_Setup_State;

CONTROLLER_STATE STATE;

void Controller_Set_Initialize()
{
	Controller_Texture();

	float Screen_W = static_cast<float>(Direct3D_GetBackBufferWidth());
	float Screen_H = static_cast<float>(Direct3D_GetBackBufferHeight());

	Controller_Setup_State = false;

	STATE = CONTROLLER_STATE::NONE;

	Alert_BG_Width	= Screen_W	* 0.75f;
	Alert_BG_Height = Screen_H * 0.75f;
	Alert_BG_X = (Screen_W  - Alert_BG_Width)  * 0.5f;
	Alert_BG_y = (Screen_H - Alert_BG_Height) * 0.5f;

	Alert_Width	 = Alert_BG_Width * 0.9f;
	Alert_Height = Alert_Width	  * 0.15f;
	Alert_X = Alert_BG_X + ((Alert_BG_Width  - Alert_Width)  * 0.5f);
	Alert_y = Alert_BG_y + ((Alert_BG_Height - Alert_Height) * 0.5f);

	Button_X = Alert_BG_Width * 0.95f;
	Button_y = Alert_BG_Height * 0.95f;
	Button_Size = Alert_BG_Width * 0.1f;

	Pressed_Time = SystemTimer_GetTime();
}

void Controller_Set_Finalize()
{
	Controller_Setup_State = false;

	Alert_BG_X	= 0;
	Alert_BG_y	= 0;
	Alert_BG_Width	= 0;
	Alert_BG_Height	= 0;

	Alert_X	= 0;
	Alert_y	= 0;
	Alert_Width  = 0;
	Alert_Height = 0;

	Button_X = 0;
	Button_y = 0;
	Button_Size = 0;
}

void Controller_Set_Update()
{
	switch (STATE)
	{
	case CONTROLLER_STATE::NONE:
		if (XKeyLogger_GetControllerConnected())
		{
			STATE = CONTROLLER_STATE::INPUT;
			Controller_Setup_State = true;
			Audio_Manager::GetInstance()->Play_SFX("Controller_Alert");
		}

		if (XKeyLogger_GetControllerDisconnected())
		{
			STATE = CONTROLLER_STATE::OUTPUT;
			Controller_Setup_State = true;
			Audio_Manager::GetInstance()->Play_SFX("Controller_Alert");
		}
		break;

	case CONTROLLER_STATE::INPUT:
		if (XKeyLogger_IsPadReleased(XINPUT_GAMEPAD_A))
		{
			Controller_Setup_State = false;
			STATE = CONTROLLER_STATE::NONE;
			Audio_Manager::GetInstance()->Play_SFX("Buffer_Denied");
		}

		if (static_cast<int>(SystemTimer_GetTime() - Pressed_Time) % 2 == 0)
		{
			Is_Pressed = false;
		}
		else if (static_cast<int>(SystemTimer_GetTime() - Pressed_Time) % 2 == 1)
		{
			Is_Pressed = true;
		}
		break;

	case CONTROLLER_STATE::OUTPUT:
		if (KeyLogger_IsReleased(KK_ENTER))
		{
			Controller_Setup_State = false;
			STATE = CONTROLLER_STATE::NONE;
			Audio_Manager::GetInstance()->Play_SFX("Buffer_Denied");
		}

		if (static_cast<int>(SystemTimer_GetTime() - Pressed_Time) % 2 == 0)
		{
			Is_Pressed = false;
		}
		else if (static_cast<int>(SystemTimer_GetTime() - Pressed_Time) % 2 == 1)
		{
			Is_Pressed = true;
		}
		break;
	}
}

void Controller_Set_Draw()
{
	switch (STATE)
	{
	case CONTROLLER_STATE::NONE:
		break;

	case CONTROLLER_STATE::INPUT:
		Sprite_Draw(UI_Controller_BG, Alert_BG_X, Alert_BG_y, Alert_BG_Width, Alert_BG_Height);
		Sprite_Draw(UI_Controller_Input, Alert_X, Alert_y, Alert_Width, Alert_Height, 0.f);

		if(Is_Pressed)
		{
			Sprite_Draw(UI_Controller_Button_Up, Button_X, Button_y, Button_Size, Button_Size, 0.f);
		}
		else
		{
			Sprite_Draw(UI_Controller_Button_Down, Button_X, Button_y, Button_Size, Button_Size, 0.f);
		}
		break;

	case CONTROLLER_STATE::OUTPUT:
		Sprite_Draw(UI_Controller_BG, Alert_BG_X, Alert_BG_y, Alert_BG_Width, Alert_BG_Height, 0.f);
		Sprite_Draw(UI_Controller_Output, Alert_X, Alert_y, Alert_Width, Alert_Height, 0.f);

		if (Is_Pressed)
		{
			Sprite_Draw(UI_Controller_Enter_Up, Button_X, Button_y, Button_Size, Button_Size, 0.f);
		}
		else
		{
			Sprite_Draw(UI_Controller_Enter_Down, Button_X, Button_y, Button_Size, Button_Size, 0.f);
		}
		break;
	}
}

bool Controller_Set_UP()
{
	return Controller_Setup_State;
}

void Controller_Texture()
{
	UI_Controller_BG			= Texture_Manager::GetInstance()->GetID("UI_Background");
	UI_Controller_Input			= Texture_Manager::GetInstance()->GetID("UI_Controller_Input");
	UI_Controller_Output		= Texture_Manager::GetInstance()->GetID("UI_Controller_Output");
	UI_Controller_Button_Up		= Texture_Manager::GetInstance()->GetID("UI_Controller_Button_Up");
	UI_Controller_Button_Down	= Texture_Manager::GetInstance()->GetID("UI_Controller_Button_Down");
	UI_Controller_Enter_Up		= Texture_Manager::GetInstance()->GetID("UI_Controller_Enter_Up");
	UI_Controller_Enter_Down	= Texture_Manager::GetInstance()->GetID("UI_Controller_Enter_Down");
}
