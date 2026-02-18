/*==============================================================================

    Manage In Game Menu [In_Game_Menu.h]

    Author : Choi HyungJoon

==============================================================================*/
#include "In_Game_Menu.h"
#include "Texture_Manager.h"
#include "Game_Screen_Manager.h"
#include "Audio_Manager.h"
#include "KeyLogger.h"
#include "Controller_Input.h"
#include "Shader_Manager.h"
#include "Palette.h"
#include "Game.h"
#include "Setting.h"
#include "Fade.h"
#include "BGM_Mixer.h"
using namespace DirectX;
using namespace PALETTE;


//----------------UI Texture----------------//
static int UI_Resume = -1; 
static int UI_Setting = -1;
static int UI_GoMain = -1; 
static int UI_BG = -1;

//----------------------POS----------------------//
static float UI_X, UI_W, UI_H;
static float Resume_Y, Setting_Y, Main_Y;
static float BG_X, BG_Y, BG_W, BG_H;

//----------------State & Data----------------//
static IN_GAME_BUFFER IG_Buffer = IN_GAME_BUFFER::NONE;

// Mouse Info
static Mouse_Info Mouse_In_Game_Menu;
static bool Is_Mouse_Left_Clicked_Prev = false;
static bool Is_Going_Main = false;

void In_Game_Menu_Initialize()
{
    In_Game_Menu_Texture();
    In_Game_Menu_Reset();

    float Screen_W = static_cast<float>(Direct3D_GetBackBufferWidth());
    float Screen_H = static_cast<float>(Direct3D_GetBackBufferHeight());

    Mouse_In_Game_Menu.Size = Screen_H * 0.05f;
    Mouse_In_Game_Menu.Prev_X = Screen_W * 0.5f;
    Mouse_In_Game_Menu.Prev_Y = Screen_H * 0.5f;

    BG_W = Screen_W * 0.8f;
    BG_H = Screen_H * 0.8f;
    BG_X = (Screen_W - BG_W) * 0.5f;
    BG_Y = (Screen_H - BG_H) * 0.5f;

    UI_H = Screen_H * 0.1f;
    UI_W = UI_H * 4.0f;
    UI_X = (Screen_W * 0.5f) - (UI_W * 0.5f);

    Main_Y    = Screen_H * 0.35f - (UI_H * 0.5f);
    Setting_Y = Screen_H * 0.5f - (UI_H * 0.5f);
    Resume_Y  = Screen_H * 0.65f - (UI_H * 0.5f);

    Is_Going_Main = false;
}

void In_Game_Menu_Finalize()
{
    In_Game_Menu_Reset();
}

void In_Game_Menu_Reset()
{
    Set_In_Game_Buffer(IN_GAME_BUFFER::NONE);
    Is_Mouse_Left_Clicked_Prev = false;
    Is_Going_Main = false;
    Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);
}

void Set_In_Game_Buffer(IN_GAME_BUFFER Buffer)
{
    IG_Buffer = Buffer;
}

IN_GAME_BUFFER Get_In_Game_Buffer()
{
    return IG_Buffer;
}


void In_Game_Menu_Update(double elapsed_time)
{
    float dt = static_cast<float>(elapsed_time);

    if (Is_Going_Main)
    {
        float currentVol = Audio_Manager::GetInstance()->Get_Target_BGM_Volume();
        float fadeOutSpeed = 1.0f / 1.5f;

        if (currentVol > 0.0f)
        {
            currentVol -= fadeOutSpeed * dt;

            if (currentVol < 0.0f) 
                currentVol = 0.0f;

            Audio_Manager::GetInstance()->Set_Target_BGM_Volume(currentVol);
        }

        if (Fade_GetState() == FADE_STATE::FINISHED_OUT)
        {
            Mixer_Init();

            Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);
            Game_Manager::GetInstance()->Update_Main_Screen(Main_Screen::MENU_SELECT);
            Game_Manager::GetInstance()->Update_Game_Select_Screen(Game_Select_Screen::G_WAIT);

            Game_Info_Reset();
            In_Game_Menu_Reset();
        }
        return;
    }

    if (KeyLogger_IsTrigger(KK_ESCAPE) || KeyLogger_IsTrigger(KK_BACK) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_B))
    {
        Audio_Manager::GetInstance()->Pause_BGM(false);
        Audio_Manager::GetInstance()->Play_SFX("Buffer_Back");
        Mouse_SetMode(MOUSE_POSITION_MODE_RELATIVE);
        Game_Manager::GetInstance()->Update_Game_Select_Screen(Game_Select_Screen::GAME_PLAYING);
        In_Game_Menu_Reset();
        return;
    }

    Mouse_State State = Mouse_Get_Prev_State(Mouse_In_Game_Menu);
    bool Mouse_Moved = Is_Mouse_Moved();

    bool Click_Trigger = (State.leftButton && !Is_Mouse_Left_Clicked_Prev);
    bool Enter_Trigger = (KeyLogger_IsTrigger(KK_ENTER) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_A));
    bool Confirm = (Click_Trigger || Enter_Trigger);

    Is_Mouse_Left_Clicked_Prev = State.leftButton;

    if (Mouse_Moved)
    {
        if (Is_Mouse_In_RECT(Mouse_In_Game_Menu.X, Mouse_In_Game_Menu.Y, UI_X, Main_Y, UI_W, UI_H))
        {
            Set_In_Game_Buffer(IN_GAME_BUFFER::MAIN_MENU);
        }
        else if (Is_Mouse_In_RECT(Mouse_In_Game_Menu.X, Mouse_In_Game_Menu.Y, UI_X, Setting_Y, UI_W, UI_H))
        {
            Set_In_Game_Buffer(IN_GAME_BUFFER::SETTING);
        }
        else if (Is_Mouse_In_RECT(Mouse_In_Game_Menu.X, Mouse_In_Game_Menu.Y, UI_X, Resume_Y, UI_W, UI_H))
        {
            Set_In_Game_Buffer(IN_GAME_BUFFER::RESUME);
        }
        else
        {
            Set_In_Game_Buffer(IN_GAME_BUFFER::WAIT);
        }
    }

    if (KeyLogger_IsTrigger(KK_W) || KeyLogger_IsTrigger(KK_UP) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_UP))
    {
        if (Get_In_Game_Buffer() == IN_GAME_BUFFER::NONE || Get_In_Game_Buffer() == IN_GAME_BUFFER::WAIT)
        {
            Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
            Set_In_Game_Buffer(IN_GAME_BUFFER::MAIN_MENU);
        }
        else
        {
            if (Get_In_Game_Buffer() == IN_GAME_BUFFER::SETTING)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                Set_In_Game_Buffer(IN_GAME_BUFFER::MAIN_MENU);
            }
            else if (Get_In_Game_Buffer() == IN_GAME_BUFFER::RESUME)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                Set_In_Game_Buffer(IN_GAME_BUFFER::SETTING);
            }
        }
    }
    else if (KeyLogger_IsTrigger(KK_S) || KeyLogger_IsTrigger(KK_DOWN) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_DOWN))
    {
        if (Get_In_Game_Buffer() == IN_GAME_BUFFER::NONE || Get_In_Game_Buffer() == IN_GAME_BUFFER::WAIT)
        {
            Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
            Set_In_Game_Buffer(IN_GAME_BUFFER::RESUME);
        }
        else
        {
            if (Get_In_Game_Buffer() == IN_GAME_BUFFER::MAIN_MENU)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                Set_In_Game_Buffer(IN_GAME_BUFFER::SETTING);
            }
            else if (Get_In_Game_Buffer() == IN_GAME_BUFFER::SETTING)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                Set_In_Game_Buffer(IN_GAME_BUFFER::RESUME);
            }
        }
    }

    if (Confirm)
    {
        Audio_Manager::GetInstance()->Play_SFX("Buffer_Select");
        switch (IG_Buffer)
        {
        case IN_GAME_BUFFER::RESUME:
            Audio_Manager::GetInstance()->Pause_BGM(false);
            Mouse_SetMode(MOUSE_POSITION_MODE_RELATIVE);
            Game_Manager::GetInstance()->Update_Game_Select_Screen(Game_Select_Screen::GAME_PLAYING);
            In_Game_Menu_Reset();
            break;

        case IN_GAME_BUFFER::SETTING:
            Game_Manager::GetInstance()->Update_Game_Select_Screen(Game_Select_Screen::GAME_SETTING);
            break;

        case IN_GAME_BUFFER::MAIN_MENU:
            Is_Going_Main = true;
            Fade_Start(1.5f, true);
            break;
        }
    }

    if (KeyLogger_IsTrigger(KK_BACK) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_START))
    {
        Mouse_SetMode(MOUSE_POSITION_MODE_RELATIVE);
        Game_Manager::GetInstance()->Update_Game_Select_Screen(Game_Select_Screen::GAME_PLAYING);
        In_Game_Menu_Reset();
    }
}

void In_Game_Menu_Draw()
{
    Direct3D_SetDepthEnable(false);
    Shader_Manager::GetInstance()->Begin2D();

    Sprite_Draw(UI_BG, BG_X, BG_Y, BG_W, BG_H);

    XMFLOAT4 Select_Color = Alpha_Origin;
    XMFLOAT4 Normal_Color = Alpha_Half;

    Sprite_Draw(UI_GoMain, UI_X, Main_Y, UI_W, UI_H, 0.0f, (IG_Buffer == IN_GAME_BUFFER::MAIN_MENU) ? Select_Color : Normal_Color);
    Sprite_Draw(UI_Setting, UI_X, Setting_Y, UI_W, UI_H, 0.0f, (IG_Buffer == IN_GAME_BUFFER::SETTING) ? Select_Color : Normal_Color);
    Sprite_Draw(UI_Resume, UI_X, Resume_Y, UI_W, UI_H, 0.0f, (IG_Buffer == IN_GAME_BUFFER::RESUME) ? Select_Color : Normal_Color);
}

Mouse_Info Get_In_Game_Mouse_POS()
{
    return Mouse_In_Game_Menu;
}

void In_Game_Menu_Texture()
{
    UI_BG = Texture_Manager::GetInstance()->GetID("UI_Background");

    UI_GoMain = Texture_Manager::GetInstance()->GetID("Main");
    UI_Setting = Texture_Manager::GetInstance()->GetID("Settings");  
    UI_Resume = Texture_Manager::GetInstance()->GetID("Return");    
}