/*==============================================================================
    
    Game Setting [Setting.cpp]

    Author : Choi HyungJoon

==============================================================================*/
#include "Setting.h"
#include "Main_Menu.h"
#include "Audio_Manager.h"
#include "Texture_Manager.h"
#include "KeyLogger.h"
#include "Controller_Input.h"
#include "Shader_Manager.h"
#include "Game_Screen_Manager.h"
#include "Player_Camera.h"
#include "Palette.h"
#include "Debug_Ostream.h"
using namespace DirectX;
using namespace PALETTE;

//----------------UI Texture----------------//
static int UI_Background = -1, UI_Pixel_White = -1;
static int Num_Arr[11], NOW_BGM_TexID, NOW_SFX_TexID;
static int UI_Run_Toggle = -1, UI_View = -1, UI_Toggle_Box_Idle = -1, UI_Toggle_Box_Check = -1;
static int UI_Toggle_Box_L = -1, UI_Toggle_Box_R = -1, UI_Button_L = -1, UI_Button_R = -1;
static int UI_BGM = -1, UI_SFX = -1,UI_Sens_Text = -1, UI_Back = -1;

//----------------------POS----------------------//
static float BG_X, BG_Y, BG_W, BG_H;

static float UI_Sens_X, UI_Sens_Y, UI_H, UI_W;

static float UI_Value_X; 
static float Slider_Bar_X, Slider_Bar_Y, Slider_Bar_W, Slider_Bar_H;
static float Slider_Box_X, Slider_Box_Y, Slider_Box_Size;

static float UI_Sprint_X, UI_Sprint_BOX_X, UI_View_X, UI_View_BOX_X, UI_Toggle_Y;

static float UI_BGM_X, UI_SFX_X, UI_Sound_Y;
static float BGM_NUM_X, SFX_NUM_X;
static float BGM_L_BUTTON_X, BGM_R_BUTTON_X;
static float SFX_L_BUTTON_X, SFX_R_BUTTON_X;

static float UI_Back_X, UI_Back_Y;

//----------------State & Data----------------//
// State Info
static SETTING_BUFFER       S_Buffer = SETTING_BUFFER::SETTING_WAIT;
static SOUND_SETTING_STATE  S_State = SOUND_SETTING_STATE::NONE;

static SOUND_SCALE_BUFFER  BGM_Volume_Level = SOUND_SCALE_BUFFER::FIVE;
static SOUND_SCALE_BUFFER  SFX_Volume_Level = SOUND_SCALE_BUFFER::FIVE;

// Sensitivity
static float Current_Sensitivity;
static const float Sens_Scale = 0.0005f;
static const float Sens_Min = 0.001f;
static const float Sens_Max = 0.1f;
static bool Is_Slider_Dragging = false;

// Setting Toggle
static bool Is_Sprint_Toggle = false;
static bool Is_Right_View = true;

static bool Reset_KeyLogger = false;

// Mouse Info
Mouse_Info Mouse_Setting;
static bool Is_Mouse_Left_Clicked_Prev = false;

// Hleper Logic : To Do >> Get Texture ID For Sound Level
static int Get_Volume_Texture_ID(SOUND_SCALE_BUFFER level);
static int Get_Volume_Texture_ID(SOUND_SCALE_BUFFER level);

void Setting_Initialize()
{
    Setting_Menu_Texture();

    Update_Setting_Buffer(SETTING_BUFFER::SETTING_NONE);
    Update_Sound_Setting_State(SOUND_SETTING_STATE::NONE);
    Update_BGM_Scale_Buffer(SOUND_SCALE_BUFFER::FIVE);
    Update_SFX_Scale_Buffer(SOUND_SCALE_BUFFER::FIVE);

    float Screen_W = static_cast<float>(Direct3D_GetBackBufferWidth());
    float Screen_H = static_cast<float>(Direct3D_GetBackBufferHeight());

    Mouse_Setting.Size = Screen_H * 0.05f;
    Mouse_Setting.Prev_X = Screen_W * 0.5f;
    Mouse_Setting.Prev_Y = Screen_H * 0.5f;

    // Backgound
    BG_W = Screen_W * 0.8f;
    BG_H = Screen_H * 0.8f;
    BG_X = (Screen_W - BG_W) * 0.5f;
    BG_Y = (Screen_H - BG_H) * 0.5f;

    // UI
    UI_H = BG_H * 0.1f;
    UI_W = UI_H * 4.0f;

    // ---------------------------------------------------
    //                   Sensitivity
    // ---------------------------------------------------
    UI_Sens_X = BG_X + BG_W * 0.15f;
    UI_Sens_Y = BG_Y + BG_H * 0.2f;

    // Slider Setup
    UI_Value_X = BG_X + BG_W * 0.65f;
    Slider_Bar_W = BG_W * 0.35f;
    Slider_Bar_H = UI_H * 0.2f;
    Slider_Bar_X = UI_Value_X - (Slider_Bar_W * 0.5f);
    Slider_Bar_Y = UI_Sens_Y + (UI_H * 0.4f);

    Slider_Box_Size = UI_H * 0.5f;
    Slider_Box_X = Slider_Bar_X + (Slider_Box_Size * 0.5f);
    Slider_Box_Y = Slider_Bar_Y + (Slider_Bar_H - Slider_Box_Size) * 0.5f;

    // ---------------------------------------------------
    //                      Toggle
    // ---------------------------------------------------
    UI_Toggle_Y = BG_Y + BG_H * 0.4f;

    // Left Center (25% Position)
    float Center_Left_X = BG_X + (BG_W * 0.25f);
	// Center (50% Position)
    float Center_X = BG_X + (BG_W * 0.5f);
    // Right Center (75% Position)
    float Center_Right_X = BG_X + (BG_W * 0.75f);
    float UI_Gap = BG_W * 0.125f;

    // Sprint (Left Area)
    UI_Sprint_X = Center_Left_X - (UI_W * 0.5f); // Text
    UI_Sprint_BOX_X = Center_Left_X + UI_Gap; // Box
    // View (Right Area)
    UI_View_X = Center_X + UI_Gap - (UI_W * 0.5f); // Text
    UI_View_BOX_X = Center_Right_X + (UI_H * 0.5f); // Box

    // ---------------------------------------------------
    //                      Sound
    // ---------------------------------------------------
    UI_Sound_Y = BG_Y + BG_H * 0.6f;
    float Arrow_Gap = UI_H * 1.2f;

    // BGM (Left Area)
    UI_BGM_X = Center_Left_X - UI_Gap - (UI_W * 0.25f);
    float BGM_Control_Center = Center_Left_X + UI_Gap - (UI_H * 0.5f);

    BGM_NUM_X = BGM_Control_Center;
    BGM_L_BUTTON_X = BGM_Control_Center - Arrow_Gap;
    BGM_R_BUTTON_X = BGM_Control_Center + Arrow_Gap;

    // SFX (Right Area)
    UI_SFX_X = Center_Right_X - UI_Gap - (UI_W * 0.5f);
    float SFX_Control_Center = Center_Right_X + UI_Gap - (UI_H * 1.5f);

    SFX_NUM_X = SFX_Control_Center;
    SFX_L_BUTTON_X = SFX_Control_Center - Arrow_Gap;
    SFX_R_BUTTON_X = SFX_Control_Center + Arrow_Gap;

    // Back
    UI_Back_X = (Screen_W * 0.5f) - (UI_W * 0.5f);
    UI_Back_Y = BG_Y + BG_H * 0.8f;

    Reset_KeyLogger = false;

    Current_Sensitivity = Get_Mouse_Sensitivity();
    Is_Slider_Dragging = false;

    Is_Sprint_Toggle = false;
    Is_Right_View = true;
}

void Setting_Finalize()
{
}

void Setting_Update(double elapsed_time)
{
    float dt = static_cast<float>(elapsed_time);

    if (Reset_KeyLogger)
    {
        if (KeyLogger_IsAnyKeyReleased() || XKeyLogger_IsAnyPadReleased())
            Reset_KeyLogger = false;
        return;
    }

    // Get Mouse Update
    Mouse_State State = Mouse_Get_Prev_State(Mouse_Setting);
    bool Mouse_Movement = Is_Mouse_Moved();
    float Mouse_X = Mouse_Setting.X;
    float Mouse_Y = Mouse_Setting.Y;

    // Slider Box POS Check
    float ratio = (Get_Mouse_Sensitivity() - Sens_Min) / (Sens_Max - Sens_Min);
    ratio = std::max(0.0f, std::min(ratio, 1.0f));
    Slider_Box_X = Slider_Bar_X + (Slider_Bar_W * ratio) - (Slider_Box_Size * 0.5f);

    bool Click_Trigger = (State.leftButton && !Is_Mouse_Left_Clicked_Prev);
    bool Enter_Trigger = (KeyLogger_IsTrigger(KK_ENTER) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_A));
    bool Confirm = (Click_Trigger || Enter_Trigger);

    Is_Mouse_Left_Clicked_Prev = State.leftButton;

    // ==============================================================================================
     //                                  Menu Navigation (None State)
     // ==============================================================================================
    if (Get_Setting_State() == SOUND_SETTING_STATE::NONE)
    {
        // 1. Mouse Hover Check
        if (Mouse_Movement)
        {
            // Row 1: Sensitivity
            if ((Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_Sens_X, UI_Sens_Y, UI_W, UI_H)) ||
                (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, Slider_Bar_X, Slider_Bar_Y, Slider_Bar_W, Slider_Bar_H)))
            {
                if (Get_Setting_Buffer() != SETTING_BUFFER::SENSITIVITY)
                {
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                    Update_Setting_Buffer(SETTING_BUFFER::SENSITIVITY);
                }
            }
            // Row 2: Sprint (Left)
            else if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_Sprint_X, UI_Toggle_Y, UI_W + UI_H * 2, UI_H))
            {
                if (Get_Setting_Buffer() != SETTING_BUFFER::SPRINT_TOGGLE)
                {
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                    Update_Setting_Buffer(SETTING_BUFFER::SPRINT_TOGGLE);
                }
            }
            // Row 2: View (Right)
            else if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_View_X, UI_Toggle_Y, UI_W + UI_H * 2, UI_H))
            {
                if (Get_Setting_Buffer() != SETTING_BUFFER::SHOULDER_VIEW)
                {
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                    Update_Setting_Buffer(SETTING_BUFFER::SHOULDER_VIEW);
                }
            }
            // Row 3: BGM (Left)
            else if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_BGM_X, UI_Sound_Y, UI_W + UI_H * 4, UI_H))
            {
                if (Get_Setting_Buffer() != SETTING_BUFFER::BGM_SETTING)
                {
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                    Update_Setting_Buffer(SETTING_BUFFER::BGM_SETTING);
                }
            }
            // Row 3: SFX (Right)
            else if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_SFX_X, UI_Sound_Y, UI_W + UI_H * 4, UI_H))
            {
                if (Get_Setting_Buffer() != SETTING_BUFFER::SFX_SETTING)
                {
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                    Update_Setting_Buffer(SETTING_BUFFER::SFX_SETTING);
                }
            }
            // Row 4: Back
            else if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_Back_X, UI_Back_Y, UI_W, UI_H))
            {
                if (Get_Setting_Buffer() != SETTING_BUFFER::SETTING_BACK)
                {
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                    Update_Setting_Buffer(SETTING_BUFFER::SETTING_BACK);
                }
            }
            else
            {
				if (Get_Setting_Buffer() != SETTING_BUFFER::SETTING_NONE)
				{
					Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
					Update_Setting_Buffer(SETTING_BUFFER::SETTING_NONE);
				}
            }
        }

        // 2. Keyboard Navigation (Up/Down/Left/Right)

        // UP Input
        if (KeyLogger_IsTrigger(KK_W) || KeyLogger_IsTrigger(KK_UP) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_UP))
        {
            Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
            switch (Get_Setting_Buffer())
            {
            case SETTING_BUFFER::SETTING_NONE:
            case SETTING_BUFFER::SETTING_WAIT:
                Update_Setting_Buffer(SETTING_BUFFER::SENSITIVITY); break;

            case SETTING_BUFFER::SPRINT_TOGGLE: // Row 2 Left -> Row 1
            case SETTING_BUFFER::SHOULDER_VIEW: // Row 2 Right -> Row 1
                Update_Setting_Buffer(SETTING_BUFFER::SENSITIVITY); break;

            case SETTING_BUFFER::BGM_SETTING:   // Row 3 Left -> Row 2 Left
                Update_Setting_Buffer(SETTING_BUFFER::SPRINT_TOGGLE); break;

            case SETTING_BUFFER::SFX_SETTING:   // Row 3 Right -> Row 2 Right
                Update_Setting_Buffer(SETTING_BUFFER::SHOULDER_VIEW); break;

            case SETTING_BUFFER::SETTING_BACK:  // Row 4 -> Row 3 Left (Default)
                Update_Setting_Buffer(SETTING_BUFFER::BGM_SETTING); break;
            }
        }
        // DOWN Input
        else if (KeyLogger_IsTrigger(KK_S) || KeyLogger_IsTrigger(KK_DOWN) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_DOWN))
        {
            Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
            switch (Get_Setting_Buffer())
            {
            case SETTING_BUFFER::SETTING_NONE:
            case SETTING_BUFFER::SETTING_WAIT:
                Update_Setting_Buffer(SETTING_BUFFER::SETTING_BACK); break;

            case SETTING_BUFFER::SENSITIVITY:   // Row 1 -> Row 2 Left (Default)
                Update_Setting_Buffer(SETTING_BUFFER::SPRINT_TOGGLE); break;

            case SETTING_BUFFER::SPRINT_TOGGLE: // Row 2 Left -> Row 3 Left
                Update_Setting_Buffer(SETTING_BUFFER::BGM_SETTING); break;

            case SETTING_BUFFER::SHOULDER_VIEW: // Row 2 Right -> Row 3 Right
                Update_Setting_Buffer(SETTING_BUFFER::SFX_SETTING); break;

            case SETTING_BUFFER::BGM_SETTING:   // Row 3 -> Row 4
            case SETTING_BUFFER::SFX_SETTING:
                Update_Setting_Buffer(SETTING_BUFFER::SETTING_BACK); break;
            }
        }
        // LEFT Input
        else if (KeyLogger_IsTrigger(KK_A) || KeyLogger_IsTrigger(KK_LEFT) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_LEFT))
        {
            if (Get_Setting_Buffer() == SETTING_BUFFER::SHOULDER_VIEW)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                Update_Setting_Buffer(SETTING_BUFFER::SPRINT_TOGGLE);
            }
            else if (Get_Setting_Buffer() == SETTING_BUFFER::SFX_SETTING)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                Update_Setting_Buffer(SETTING_BUFFER::BGM_SETTING);
            }
        }
        // RIGHT Input
        else if (KeyLogger_IsTrigger(KK_D) || KeyLogger_IsTrigger(KK_RIGHT) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_RIGHT))
        {
            if (Get_Setting_Buffer() == SETTING_BUFFER::SPRINT_TOGGLE)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                Update_Setting_Buffer(SETTING_BUFFER::SHOULDER_VIEW);
            }
            else if (Get_Setting_Buffer() == SETTING_BUFFER::BGM_SETTING)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                Update_Setting_Buffer(SETTING_BUFFER::SFX_SETTING);
            }
        }

        // 3. Selection Logic (Confirm)
        if (Confirm)
        {
            bool Is_Valid_Selection = (Get_Setting_Buffer() != SETTING_BUFFER::SETTING_NONE && Get_Setting_Buffer() != SETTING_BUFFER::SETTING_WAIT);

            if (Is_Valid_Selection)
            {
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Select");

                switch (Get_Setting_Buffer())
                {
                case SETTING_BUFFER::SENSITIVITY:
                    Update_Sound_Setting_State(SOUND_SETTING_STATE::SENSITIVITY_SETTING);
                    break;

                case SETTING_BUFFER::SPRINT_TOGGLE:
                    Update_Sound_Setting_State(SOUND_SETTING_STATE::SPRINT_SETTING);
                    break;

                case SETTING_BUFFER::SHOULDER_VIEW:
                    Update_Sound_Setting_State(SOUND_SETTING_STATE::VIEW_SETTING);
                    break;

                case SETTING_BUFFER::BGM_SETTING:
                    Update_Sound_Setting_State(SOUND_SETTING_STATE::BGM_SETTING);
                    break;

                case SETTING_BUFFER::SFX_SETTING:
                    Update_Sound_Setting_State(SOUND_SETTING_STATE::SFX_SETTING);
                    break;

                case SETTING_BUFFER::SETTING_BACK:
                    if (Game_Manager::GetInstance()->Get_Current_Main_Screen() == Main_Screen::SELECT_GAME)
                    {
                        Game_Manager::GetInstance()->Update_Game_Select_Screen(Game_Select_Screen::GAME_IN_GAME_MENU);
                    }
                    else
                    {
                        Game_Manager::GetInstance()->Update_Sub_Screen(Sub_Screen::S_DONE);
                    }
                    Update_Setting_Buffer(SETTING_BUFFER::SETTING_NONE);
                    break;
                }
            }
        }
    }
    // ==============================================================================================
    //                                  Edit Modes (Inner Logic)
    // ==============================================================================================
    else
    {
        // Global Back Logic for All Edit Modes
        if (KeyLogger_IsTrigger(KK_BACK) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_B) || State.rightButton)
        {
            Audio_Manager::GetInstance()->Play_SFX("Buffer_Select");
            Update_Sound_Setting_State(SOUND_SETTING_STATE::NONE);
            Is_Slider_Dragging = false;
            return;
        }

        switch (Get_Setting_State())
        {
            // ------------------ Sensitivity Edit ------------------
        case SOUND_SETTING_STATE::SENSITIVITY_SETTING:
        {
            // Exit if clicked outside slider area
            if (Click_Trigger)
            {
                if (!Is_Mouse_In_RECT(Mouse_X, Mouse_Y, Slider_Bar_X - 10, Slider_Bar_Y - 20, Slider_Bar_W + 20, UI_H))
                {
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Select");
                    Update_Sound_Setting_State(SOUND_SETTING_STATE::NONE);
                    Is_Slider_Dragging = false;
                    return;
                }
            }

            // Keyboard Adjustment
            if (KeyLogger_IsTrigger(KK_A) || KeyLogger_IsTrigger(KK_LEFT) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_LEFT))
            {
                Current_Sensitivity -= Sens_Scale;
            }
            else if (KeyLogger_IsTrigger(KK_D) || KeyLogger_IsTrigger(KK_RIGHT) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_RIGHT))
            {
                Current_Sensitivity += Sens_Scale;
            }

            // Mouse Dragging
            if (Click_Trigger && Is_Mouse_In_RECT(Mouse_X, Mouse_Y, Slider_Bar_X - 10, Slider_Bar_Y - 20, Slider_Bar_W + 20, UI_H))
            {
                Is_Slider_Dragging = true;
            }

            if (Is_Slider_Dragging && KeyLogger_IsMousePressed(Mouse_Button::LEFT))
            {
                float newRatio = (Mouse_X - Slider_Bar_X) / Slider_Bar_W;
                newRatio = std::max(0.0f, std::min(newRatio, 1.0f));
                Current_Sensitivity = Sens_Min + newRatio * (Sens_Max - Sens_Min);
            }
            else
            {
                Is_Slider_Dragging = false;
            }

            // Clamp & Apply
            Current_Sensitivity = std::max(Sens_Min, std::min(Current_Sensitivity, Sens_Max));
            Set_Mouse_Sensitivity(Current_Sensitivity);
        }
        break;

        // ------------------ Sprint Edit (Toggle) ------------------
        case SOUND_SETTING_STATE::SPRINT_SETTING:
        {
            // Toggle Logic (Enter or Click Box)
            if (Confirm)
            {
                // Box Click Check
                if (Click_Trigger)
                {
                    if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_Sprint_BOX_X, UI_Toggle_Y, UI_H, UI_H) ||
                        Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_Sprint_X, UI_Toggle_Y, UI_W + UI_H * 2, UI_H))
                    {
                        Is_Sprint_Toggle = !Is_Sprint_Toggle;
                        Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                    }
                    else
                    {
                        // Clicked outside -> Exit
                        Audio_Manager::GetInstance()->Play_SFX("Buffer_Select");
                        Update_Sound_Setting_State(SOUND_SETTING_STATE::NONE);
                    }
                }
                else // Enter Key
                {
                    Is_Sprint_Toggle = !Is_Sprint_Toggle;
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                }
            }
        }
        break;

        // ------------------ View Edit (Swap L/R) ------------------
        case SOUND_SETTING_STATE::VIEW_SETTING:
        {
            // Toggle Logic
            if (Confirm)
            {
                if (Click_Trigger)
                {
                    if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_View_BOX_X, UI_Toggle_Y, UI_H, UI_H) ||
                        Is_Mouse_In_RECT(Mouse_X, Mouse_Y, UI_View_X, UI_Toggle_Y, UI_W + UI_H * 2, UI_H))
                    {
                        Is_Right_View = !Is_Right_View;
                        Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                    }
                    else
                    {
                        Audio_Manager::GetInstance()->Play_SFX("Buffer_Select");
                        Update_Sound_Setting_State(SOUND_SETTING_STATE::NONE);
                    }
                }
                else // Enter / Arrow Keys
                {
                    // Allow Arrows to swap too
                    Is_Right_View = !Is_Right_View;
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                }
            }
            // Arrow Keys for View Swap
            else if (KeyLogger_IsTrigger(KK_A) || KeyLogger_IsTrigger(KK_LEFT) || KeyLogger_IsTrigger(KK_D) || KeyLogger_IsTrigger(KK_RIGHT))
            {
                Is_Right_View = !Is_Right_View;
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
            }
        }
        break;

        // ------------------ BGM & SFX Edit ------------------
        case SOUND_SETTING_STATE::BGM_SETTING:
        case SOUND_SETTING_STATE::SFX_SETTING:
        {
            bool Is_BGM_Mode = (Get_Setting_State() == SOUND_SETTING_STATE::BGM_SETTING);

            // Set Target Vars
            int currentVal = static_cast<int>(Is_BGM_Mode ? Get_BGM_Scale_Buffer() : Get_SFX_Scale_Buffer());
            float L_Btn_X = Is_BGM_Mode ? BGM_L_BUTTON_X : SFX_L_BUTTON_X;
            float R_Btn_X = Is_BGM_Mode ? BGM_R_BUTTON_X : SFX_R_BUTTON_X;

            bool Changed = false;

            // Keyboard
            if (KeyLogger_IsTrigger(KK_A) || KeyLogger_IsTrigger(KK_LEFT) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_LEFT))
            {
                currentVal--;
                Changed = true;
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
            }
            else if (KeyLogger_IsTrigger(KK_D) || KeyLogger_IsTrigger(KK_RIGHT) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_DPAD_RIGHT))
            {
                currentVal++;
                Changed = true;
                Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
            }

            // Mouse
            if (Click_Trigger)
            {
                if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, L_Btn_X, UI_Sound_Y, UI_H, UI_H))
                {
                    currentVal--;
                    Changed = true;
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                }
                else if (Is_Mouse_In_RECT(Mouse_X, Mouse_Y, R_Btn_X, UI_Sound_Y, UI_H, UI_H))
                {
                    currentVal++;
                    Changed = true;
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Move");
                }
                else
                {
                    // Clicked outside arrows -> Exit
                    Audio_Manager::GetInstance()->Play_SFX("Buffer_Select");
                    Update_Sound_Setting_State(SOUND_SETTING_STATE::NONE);
                }
            }

            // Apply
            if (Changed)
            {
                currentVal = std::max(0, std::min(currentVal, 10));
                float vol = static_cast<float>(currentVal) / 10.0f;

                if (Is_BGM_Mode)
                {
                    Update_BGM_Scale_Buffer(static_cast<SOUND_SCALE_BUFFER>(currentVal));
                    Audio_Manager::GetInstance()->Set_Target_BGM_Volume(vol);
                }
                else
                {
                    Update_SFX_Scale_Buffer(static_cast<SOUND_SCALE_BUFFER>(currentVal));
                    Audio_Manager::GetInstance()->Set_Target_SFX_Volume(vol);
                }
            }
        }
        break;
        }
    }
}

void Setting_Draw()
{
    Direct3D_SetDepthEnable(false);
    Shader_Manager::GetInstance()->Begin2D();

    // Common Colors
    XMFLOAT4 Select_Color = Alpha_Origin;
    XMFLOAT4 Normal_Color = Alpha_Half;

    // Background Panel
    Sprite_Draw(UI_Background, BG_X, BG_Y, BG_W, BG_H, 0.0f, Alpha_Origin);

    // ------------------------------------------------------------
    // 1. Sensitivity Slider
    // ------------------------------------------------------------
    bool Is_Sens_Focus = (Get_Setting_Buffer() == SETTING_BUFFER::SENSITIVITY || Get_Setting_State() == SOUND_SETTING_STATE::SENSITIVITY_SETTING);
    // Draw Text (Needs Texture)
    if (UI_Sens_Text != -1)
    {
        Sprite_Draw(UI_Sens_Text, UI_Sens_X, UI_Sens_Y, UI_W, UI_H, 0.0f, Is_Sens_Focus ? Select_Color : Normal_Color);
    }

    // Slider Bar & Box
    Sprite_Draw(UI_Pixel_White, Slider_Bar_X, Slider_Bar_Y, Slider_Bar_W, Slider_Bar_H, 0.0f, Light_Gray);
    Sprite_Draw(UI_Pixel_White, Slider_Box_X, Slider_Box_Y, Slider_Box_Size, Slider_Box_Size, 45.0f, Is_Sens_Focus ? Select_Color : Normal_Color);

    // ------------------------------------------------------------
    // 2. Row 2: Toggle (Sprint & View)
    // ------------------------------------------------------------

    // A. Sprint (Left)
    bool Is_Sprint_Focus = (Get_Setting_Buffer() == SETTING_BUFFER::SPRINT_TOGGLE || Get_Setting_State() == SOUND_SETTING_STATE::SPRINT_SETTING);
    XMFLOAT4 Sprint_Color = Is_Sprint_Focus ? Select_Color : Normal_Color;

    // Text (Sprint) And Check Box
    int Sprint_Tex = Is_Sprint_Toggle ? UI_Toggle_Box_Check : UI_Toggle_Box_Idle;
    if (UI_Run_Toggle != -1 || Sprint_Tex != -1)
    {
        Sprite_Draw(UI_Run_Toggle, UI_Sprint_X, UI_Toggle_Y, UI_W, UI_H, 0.0f, Sprint_Color);
        Sprite_Draw(Sprint_Tex, UI_Sprint_BOX_X, UI_Toggle_Y, UI_H, UI_H, 0.0f, Sprint_Color);
    }

    // B. View (Right)
    bool Is_View_Focus = (Get_Setting_Buffer() == SETTING_BUFFER::SHOULDER_VIEW || Get_Setting_State() == SOUND_SETTING_STATE::VIEW_SETTING);
    XMFLOAT4 View_Color = Is_View_Focus ? Select_Color : Normal_Color;

    // Text And View Icon (L or R)
    int View_Tex = Is_Right_View ? UI_Toggle_Box_R : UI_Toggle_Box_L;
    if (UI_View != -1 || View_Tex != -1)
    {
        Sprite_Draw(UI_View, UI_View_X, UI_Toggle_Y, UI_W, UI_H, 0.0f, View_Color);
        Sprite_Draw(View_Tex, UI_View_BOX_X, UI_Toggle_Y, UI_H, UI_H, 0.0f, View_Color);
    }

    // ------------------------------------------------------------
    // 3. Row 3: Audio (BGM & SFX)
    // ------------------------------------------------------------

    // A. BGM (Left)
    bool Is_BGM_Focus = (Get_Setting_Buffer() == SETTING_BUFFER::BGM_SETTING || Get_Setting_State() == SOUND_SETTING_STATE::BGM_SETTING);
    XMFLOAT4 BGM_Color = Is_BGM_Focus ? Select_Color : Normal_Color;

    // Label
    Sprite_Draw(UI_BGM, UI_BGM_X, UI_Sound_Y, UI_W, UI_H, 0.0f, BGM_Color);
    // Arrows
    Sprite_Draw(UI_Button_L, BGM_L_BUTTON_X, UI_Sound_Y, UI_H, UI_H, 0.0f, BGM_Color);
    Sprite_Draw(UI_Button_R, BGM_R_BUTTON_X, UI_Sound_Y, UI_H, UI_H, 0.0f, BGM_Color);
    // Number
    int bgmIdx = static_cast<int>(Get_BGM_Scale_Buffer());
    if (bgmIdx >= 0 && bgmIdx <= 10)
    {
        Sprite_Draw(Num_Arr[bgmIdx], BGM_NUM_X, UI_Sound_Y, UI_H, UI_H, 0.0f, BGM_Color);
    }

    // B. SFX (Right)
    bool Is_SFX_Focus = (Get_Setting_Buffer() == SETTING_BUFFER::SFX_SETTING || Get_Setting_State() == SOUND_SETTING_STATE::SFX_SETTING);
    XMFLOAT4 SFX_Color = Is_SFX_Focus ? Select_Color : Normal_Color;

    // Label
    Sprite_Draw(UI_SFX, UI_SFX_X, UI_Sound_Y, UI_W, UI_H, 0.0f, SFX_Color);
    // Arrows
    Sprite_Draw(UI_Button_L, SFX_L_BUTTON_X, UI_Sound_Y, UI_H, UI_H, 0.0f, SFX_Color);
    Sprite_Draw(UI_Button_R, SFX_R_BUTTON_X, UI_Sound_Y, UI_H, UI_H, 0.0f, SFX_Color);
    // Number
    int sfxIdx = static_cast<int>(Get_SFX_Scale_Buffer());
    if (sfxIdx >= 0 && sfxIdx <= 10)
    {
        Sprite_Draw(Num_Arr[sfxIdx], SFX_NUM_X, UI_Sound_Y, UI_H, UI_H, 0.0f, SFX_Color);
    }

    // ------------------------------------------------------------
    // 4. Back Button
    // ------------------------------------------------------------
    bool Is_Back_Focus = (Get_Setting_Buffer() == SETTING_BUFFER::SETTING_BACK);
    Sprite_Draw(UI_Back, UI_Back_X, UI_Back_Y, UI_W, UI_H, 0.0f, Is_Back_Focus ? Select_Color : Normal_Color);
}

void Update_Setting_Buffer(SETTING_BUFFER Buffer)
{
    S_Buffer = Buffer;
}

SETTING_BUFFER Get_Setting_Buffer()
{
    return S_Buffer;
}

void Update_Sound_Setting_State(SOUND_SETTING_STATE State)
{
    S_State = State;
}

SOUND_SETTING_STATE Get_Setting_State()
{
    return S_State;
}

void Update_BGM_Scale_Buffer(SOUND_SCALE_BUFFER BGM_Scale)
{
    BGM_Volume_Level = BGM_Scale;
}

SOUND_SCALE_BUFFER Get_BGM_Scale_Buffer()
{
    return static_cast<SOUND_SCALE_BUFFER>(BGM_Volume_Level);
}

void Update_SFX_Scale_Buffer(SOUND_SCALE_BUFFER SFX_Scale)
{
    SFX_Volume_Level = SFX_Scale;
}

SOUND_SCALE_BUFFER Get_SFX_Scale_Buffer()
{
    return static_cast<SOUND_SCALE_BUFFER>(SFX_Volume_Level);
}

bool Setting_Get_Sprint_Type()
{
    return Is_Sprint_Toggle;
}

bool Setting_Get_View_Type()
{
    return Is_Right_View;
}

Mouse_Info Get_Setting_Menu_Mouse_POS()
{
    return Mouse_Setting;
}

void Setting_Menu_Texture()
{
    //----------------Number Texture----------------//
    Num_Arr[0]  = Texture_Manager::GetInstance()->GetID("UI_Num_MIN");
    Num_Arr[1]  = Texture_Manager::GetInstance()->GetID("UI_Num_1");
    Num_Arr[2]  = Texture_Manager::GetInstance()->GetID("UI_Num_2");
    Num_Arr[3]  = Texture_Manager::GetInstance()->GetID("UI_Num_3");
    Num_Arr[4]  = Texture_Manager::GetInstance()->GetID("UI_Num_4");
    Num_Arr[5]  = Texture_Manager::GetInstance()->GetID("UI_Num_5");
    Num_Arr[6]  = Texture_Manager::GetInstance()->GetID("UI_Num_6");
    Num_Arr[7]  = Texture_Manager::GetInstance()->GetID("UI_Num_7");
    Num_Arr[8]  = Texture_Manager::GetInstance()->GetID("UI_Num_8");
    Num_Arr[9]  = Texture_Manager::GetInstance()->GetID("UI_Num_9");
    Num_Arr[10] = Texture_Manager::GetInstance()->GetID("UI_Num_MAX");
    UI_Button_L = Texture_Manager::GetInstance()->GetID("UI_Num_Button_L");
    UI_Button_R = Texture_Manager::GetInstance()->GetID("UI_Num_Button_R");

    //------------------UI Texture------------------//
    UI_BGM       = Texture_Manager::GetInstance()->GetID("UI_BGM");
    UI_SFX       = Texture_Manager::GetInstance()->GetID("UI_SFX");
    UI_Sens_Text = Texture_Manager::GetInstance()->GetID("UI_Sensitivity"); 
    UI_Back      = Texture_Manager::GetInstance()->GetID("UI_Back");
    UI_Run_Toggle = Texture_Manager::GetInstance()->GetID("UI_Run_Toggle");
    UI_View       = Texture_Manager::GetInstance()->GetID("UI_View");

    UI_Background  = Texture_Manager::GetInstance()->GetID("UI_Background");
    UI_Pixel_White = Texture_Manager::GetInstance()->GetID("Pixel_Withe");
    UI_Toggle_Box_Idle  = Texture_Manager::GetInstance()->GetID("UI_Weapon_Box");
    UI_Toggle_Box_Check = Texture_Manager::GetInstance()->GetID("UI_Toggle_Check");
    UI_Toggle_Box_L = Texture_Manager::GetInstance()->GetID("UI_Toggle_L");
    UI_Toggle_Box_R = Texture_Manager::GetInstance()->GetID("UI_Toggle_R");
}

static int Get_Volume_Texture_ID(SOUND_SCALE_BUFFER level)
{
    int idx = static_cast<int>(level);

    if (idx < 0)
    {
        idx = 0;
    }

    if (idx > 10)
    {
        idx = 10;
    }

    return Num_Arr[idx];
}