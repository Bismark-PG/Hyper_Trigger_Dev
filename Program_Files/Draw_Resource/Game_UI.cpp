/*==============================================================================

	Manage Draw Resource For Game UI [Game_UI.h]

	Author : Choi HyungJoon

==============================================================================*/
#include "Game_UI.h"
#include "Sprite.h"
#include "Texture_Manager.h"
#include <direct3d.h>
#include "Player.h"          
#include "Resource_Manager.h"
#include "Shader_Manager.h"
#include "Palette.h"
#include "Weapon_System.h"
#include <string>
#include <vector>
#include <cmath>
#include <debug_ostream.h>
#include "Player_Camera.h"
#include "KeyLogger.h"
using namespace PALETTE;

// --- Texture ID ---
static int UI_Normal_Aim = -1, UI_Zoom_Aim = -1, UI_Damage_Red = -1;
static int UI_HP_Bar = -1, UI_HP_Fill = -1, UI_EXP_Bar = -1, UI_EXP_Fill = -1;

// Weapon
static int UI_Weapon_BG = -1, UI_Current_Weapon = -1, UI_Next_Weapon_1 = -1, UI_Next_Weapon_2 = -1;
static int UI_Ammo_BG = -1, UI_Icon_HG = -1, UI_Icon_AR = -1, UI_Icon_MG = -1, UI_Icon_G = -1;
static int UI_Reload = -1, UI_Prompt = -1;

// Numbers
static int UI_Num[10] = { -1 }, UI_Num_Slash = -1, UI_Percentage = -1;

// --- POS ---
// Aim
static float Aim_X, Aim_Y, Aim_Size, Aim_Radian;
constexpr float Aim_Rotation_Amount = 0.0f;
static bool Aiming_Now = false;

// UI Bar Layout
static float Bar_W, Bar_H, Bar_Size;

// HP Bar Area
static float HP_BG_X, HP_BG_Y;
static float HP_Min_X, HP_Max_X;

// EXP Bar Area
static float EXP_BG_X, EXP_BG_Y;
static float EXP_Min_X, EXP_Max_X;

// Ratios
static float Current_HP_Ratio = 1.0f, Current_EXP_Ratio = 0.0f;

// Reload UI Parameter
static float Reload_Rotation = 0.0f;
static float UI_Reload_Timer = 0.0f;
constexpr float Reload_Base_Speed = DirectX::XM_2PI * 4.0f;

// Ammo
static const std::deque<WeaponState>* P_Inventory = nullptr;
static float Ammo_Box_X, Ammo_Box_Y, Ammo_Box_W, Ammo_Box_H;
static float Cur_Weapon_X, Cur_Weapon_Y, Cur_Weapon_Size;
static float Next_Weapon_1_X, Next_Weapon_1_Y, Next_Weapon_Size, Next_Weapon_Offset;
static float Next_Weapon_2_X, Next_Weapon_2_Y;

// Weapon Pick UP
static float Pick_X, Pick_Y, Pick_W, Pick_H;

// Damaged
static float Damage_Effect_Timer = 0.0f;
static bool  Is_Damage_Effect_On = false;
static float Damage_Alpha = 0.0f;
constexpr float Max_Damage_Alpha = 0.1f; 

void Draw_Number_String(const std::string& str, float startX, float startY, float width, float height);
static int Get_Weapon_Icon_ID(WeaponType type);
void Get_Prompt_Tex_UI();
bool Get_Ready_Prompt_UI();

void Game_UI_Initialize()
{
	Game_UI_Texture();

	float Screen_W = static_cast<float>(Direct3D_GetBackBufferWidth());
	float Screen_H = static_cast<float>(Direct3D_GetBackBufferHeight());

    // 1. Aim
    Aim_Size = Screen_H * 0.125f;
    Aim_X = (Screen_W * 0.5f) - (Aim_Size * 0.5f);
    Aim_Y = (Screen_H * 0.5f) - (Aim_Size * 0.5f);
    Aim_Radian = 0.0f;

    // 2. Bar Dimensions
    Bar_Size = Screen_H * 0.05f;
    Bar_W = (Bar_Size * 10.0f);
    Bar_H = (Bar_Size * 1.5f);

    // Set Fill
    float Padding_Big = 0.025f;
    float Padding_Small = 0.01f;

    // 3. EXP Bar
    EXP_BG_X = Screen_W * 0.01f;
    EXP_BG_Y = Screen_H - Bar_H - EXP_BG_X;

    // 4. HP Bar
    HP_BG_X = EXP_BG_X;
    HP_BG_Y = EXP_BG_Y - Bar_H;

    // HP: [Big Pad] [Fill ----->] [Small Pad]
    HP_Min_X = HP_BG_X + (Bar_W * Padding_Big);
    HP_Max_X = HP_BG_X + (Bar_W * (1.0f - Padding_Small));
    EXP_Min_X = EXP_BG_X + (Bar_W * Padding_Big);
    EXP_Max_X = EXP_BG_X + (Bar_W * (1.0f - Padding_Small));

    // If Want R To L Use This
    // EXP: [Small Pad] [<----- Fill] [Big Pad]
    // EXP_Min_X = EXP_BG_X + (Bar_W * Padding_Small);
    // EXP_Max_X = EXP_BG_X + (Bar_W * (1.0f - Padding_Big));

    // 5. Ammo
    Ammo_Box_W = Bar_W * 0.6f;
    Ammo_Box_H = EXP_BG_Y - HP_BG_Y; 
    Ammo_Box_X = Screen_W - EXP_BG_X - Ammo_Box_W;
    Ammo_Box_Y = EXP_BG_Y;

    // 6. Current Weapon
    Cur_Weapon_Size = Ammo_Box_H;
    Cur_Weapon_X = Ammo_Box_X - (Cur_Weapon_Size * 1.2f);
    Cur_Weapon_Y = Ammo_Box_Y;

    // 7. Next Weapon
    Next_Weapon_Size = Ammo_Box_H * 0.5f;
    Next_Weapon_Offset = (Ammo_Box_W * 0.5f) * 0.5f;

    // Next 1
    Next_Weapon_1_X = Ammo_Box_X + Next_Weapon_Offset - (Next_Weapon_Size * 0.5f);
    Next_Weapon_1_Y = Ammo_Box_Y - (Next_Weapon_Size * 1.2f);

    // Next 2
    Next_Weapon_2_X = Ammo_Box_X + (Next_Weapon_Offset * 1.5f) + (Next_Weapon_Size * 0.5f);;
    Next_Weapon_2_Y = Next_Weapon_1_Y;

    // 8. Weapon Pick Up Prompt
    Pick_W = Screen_W * 0.3f, Pick_H = Screen_H * 0.1f;
    Pick_X = (Screen_W * 0.5f) - (Pick_W * 0.5f);
    Pick_Y = (Screen_H * 0.85f) - (Pick_H * 0.5f);

    // 9. Damaged Overlay
    Is_Damage_Effect_On = false;
    Damage_Effect_Timer = 0.0f;
    Damage_Alpha = 0.0f;

    // 10. Clear Inventory Pointer
    P_Inventory = nullptr;

    // 11. Reload UI Parameter
    UI_Reload_Timer = 0.0f;
    Reload_Rotation = 0.0f;
}

void Game_UI_Finalize()
{
}

void Game_UI_Update(double elapsed_time)
{
    float dt = static_cast<float>(elapsed_time);

    Get_Prompt_Tex_UI();

    // HP Ratio Calculation
    int curHP = Player_Get_HP();
    int maxHP = Player_Get_MaxHP();
    if (maxHP > 0)
    {
        Current_HP_Ratio = static_cast<float>(curHP) / static_cast<float>(maxHP);
    }
    else
    {
        Current_HP_Ratio = 0.0f;
    }

    // EXP Ratio Calculation
    Current_EXP_Ratio = Resource_Manager::GetInstance().Get_Exp_Ratio();

    // Clamp (0.0 ~ 1.0)
    Current_HP_Ratio  = std::max(0.0f, std::min(Current_HP_Ratio, 1.0f));
    Current_EXP_Ratio = std::max(0.0f, std::min(Current_EXP_Ratio, 1.0f));

    // Get Weapon Info
    P_Inventory = &Weapon_System::GetInstance().GetInventory();

    if (P_Inventory != nullptr)
    {
        size_t size = P_Inventory->size();

        if (size > 0)
        {
            UI_Current_Weapon = Get_Weapon_Icon_ID(P_Inventory->at(0).Spec.Type);
        }
        else
        {
            UI_Current_Weapon = -1;
        }

        if (size > 1)
        {
            UI_Next_Weapon_1 = Get_Weapon_Icon_ID(P_Inventory->at(1).Spec.Type);
        }
        else
        {
            UI_Next_Weapon_1 = -1;
        }

        if (size > 2)
        {
            UI_Next_Weapon_2 = Get_Weapon_Icon_ID(P_Inventory->at(2).Spec.Type);
        }
        else
        {
            UI_Next_Weapon_2 = -1;
        }
    }

    bool Reloading = Weapon_System::GetInstance().Is_Reloading();
    bool Switching = Weapon_System::GetInstance().Is_Switching();

    if (Reloading || Switching)
    {
        float Spin_Speed = DirectX::XM_2PI * 10.0f;

        Reload_Rotation -= Spin_Speed * dt;

   //     float Ratio = 0.0f;

   //     if (Reloading)
   //     {
   //         // 0.0(Start) ~ 1.0(End)
   //         Ratio = Weapon_System::GetInstance().Get_Reload_State();
   //     }
   //     else
   //     {
   //         // 0.0(Start) ~ 1.0(End)
   //         Ratio = Weapon_System::GetInstance().Get_Switching_State();
   //     }

   //     // Safety Clamp
   //     if (Ratio > 1.0f) Ratio = 1.0f;

   //     if (Ratio > 0.0f)
   //     {
			//// Ease In-Out Cubic (0.0 ~ 1.0)
   //         float Accelerated_Ratio = Ratio * Ratio;

			//// Angle = Ratio * Reload_Base_Speed(4 Full Rotation Per Second)
   //         Reload_Rotation = Accelerated_Ratio * Reload_Base_Speed;
   //     }
    }
    else
    {
        Reload_Rotation = 0.0f;
    }

    if (Is_Damage_Effect_On)
    {
        Damage_Effect_Timer += dt;

        if (Damage_Effect_Timer <= 0.0f)
        {
            float ratio = Damage_Effect_Timer / 0.5f;
            Damage_Alpha = ratio * Max_Damage_Alpha;
        }
        else if (Damage_Effect_Timer <= 0.5f)
        {
            float ratio = (Damage_Effect_Timer - 0.25f) / 0.5f;
            Damage_Alpha = Max_Damage_Alpha * (1.0f - ratio);
        }
        else
        {
            Is_Damage_Effect_On = false;
            Damage_Alpha = 0.0f;
        }
    }
}

void Game_UI_Draw()
{
    Direct3D_SetDepthEnable(false);
    Shader_Manager::GetInstance()->Begin2D();
    float Texture_Padding_Ratio = 0.25f;

    // -----------------------------------------------------------
    // 1-1. HP Bar (Left -> Right)
    // -----------------------------------------------------------
    Sprite_Draw(UI_HP_Bar, HP_BG_X, HP_BG_Y, Bar_W, Bar_H);

    if (Current_HP_Ratio > 0.0f)
    {
        float Tex_W = static_cast<float>(Texture_Manager::GetInstance()->Get_Width(UI_HP_Fill));
        float Tex_H = static_cast<float>(Texture_Manager::GetInstance()->Get_Height(UI_HP_Fill));

        float Adjusted_Ratio = Texture_Padding_Ratio + (1.0f - Texture_Padding_Ratio) * Current_HP_Ratio;

        float Draw_Width = (HP_Max_X - HP_Min_X) * Adjusted_Ratio;
        float UV_Width = Tex_W * Adjusted_Ratio;

        Sprite_UV_Draw(UI_HP_Fill, HP_Min_X, HP_BG_Y, Draw_Width, Bar_H, A_Zero, A_Zero, UV_Width, Tex_H);
    }

    // -----------------------------------------------------------
	// 1-2. Draw HP Text (Current HP / Max HP)
    // -----------------------------------------------------------
    int curHP = Player_Get_HP(), maxHP = Player_Get_MaxHP();
    char HP_Buffer[32];
    sprintf_s(HP_Buffer, "%d / %d", curHP, maxHP);

	// Number Size
    float Num_H = Bar_H * 0.325f, Num_W = Num_H * 0.5f;

	// Total Text Width = (Count) * (Width) * (0.8f)
    size_t HP_Len = strlen(HP_Buffer);
    float Total_HP_Text_W = HP_Len * (Num_W * 0.8f);

	// Get Center POS of HP Bar
    float HP_Center_X = HP_BG_X + (Bar_W * 0.5f), HP_Center_Y = HP_BG_Y + (Bar_H - Num_H) * 0.5f;
    float HP_Start_X = HP_Center_X - (Total_HP_Text_W * 0.5f);

    Draw_Number_String(HP_Buffer, HP_Start_X, HP_Center_Y, Num_W, Num_H);

    // -----------------------------------------------------------
    // 2-1. EXP Bar (Left -> Right)
    // -----------------------------------------------------------
    Sprite_Draw(UI_EXP_Bar, EXP_BG_X, EXP_BG_Y, Bar_W, Bar_H);

    if (Current_EXP_Ratio > 0.0f)
    {
        float Tex_W = static_cast<float>(Texture_Manager::GetInstance()->Get_Width(UI_EXP_Fill));
        float Tex_H = static_cast<float>(Texture_Manager::GetInstance()->Get_Height(UI_EXP_Fill));

        float Adjusted_Ratio = Texture_Padding_Ratio + (1.0f - Texture_Padding_Ratio) * Current_EXP_Ratio;

        float Draw_Width = (EXP_Max_X - EXP_Min_X) * Adjusted_Ratio;
        float UV_Width = Tex_W * Adjusted_Ratio;

        Sprite_UV_Draw(UI_EXP_Fill, EXP_Min_X, EXP_BG_Y, Draw_Width, Bar_H, A_Zero, A_Zero, UV_Width, Tex_H);
    }

    // -----------------------------------------------------------
	// 2-2. EXP Text (Percentage)
    // -----------------------------------------------------------
    int ExpPercent = static_cast<int>(Current_EXP_Ratio * 100.0f);
    char EXP_Buffer[32];
    sprintf_s(EXP_Buffer, "%d%%", ExpPercent);

	// Total Text Width = (Count) * (Width) * (0.8f)
    size_t EXP_Len = strlen(EXP_Buffer);
    float Total_EXP_Text_W = EXP_Len * (Num_W * 0.8f);

	// Get Center POS of EXP Bar
    float EXP_Center_X = EXP_BG_X + (Bar_W * 0.5f), EXP_Center_Y = EXP_BG_Y + (Bar_H - Num_H) * 0.5f;
    float EXP_Start_X = EXP_Center_X - (Total_EXP_Text_W * 0.5f);

    Draw_Number_String(EXP_Buffer, EXP_Start_X, EXP_Center_Y, Num_W, Num_H);

    // -----------------------------------------------------------
    // 3. Weapon UI (Right Side)
    // -----------------------------------------------------------

    // Backgrounds (Always Draw Boxes)
    // Ammo Box
    Sprite_Draw(UI_Ammo_BG, Ammo_Box_X, Ammo_Box_Y, Ammo_Box_W, Ammo_Box_H);
    // Cur Weapon Box
    Sprite_Draw(UI_Weapon_BG, Cur_Weapon_X, Cur_Weapon_Y, Cur_Weapon_Size, Cur_Weapon_Size);
    // Next Weapon Boxes
    Sprite_Draw(UI_Weapon_BG, Next_Weapon_1_X, Next_Weapon_1_Y, Next_Weapon_Size, Next_Weapon_Size);
    Sprite_Draw(UI_Weapon_BG, Next_Weapon_2_X, Next_Weapon_2_Y, Next_Weapon_Size, Next_Weapon_Size);

    if (Weapon_System::GetInstance().HasWeapon())
    {
        // A. Current Weapon (Index 0)
        if (P_Inventory != nullptr && !P_Inventory->empty())
        {
            if (UI_Current_Weapon != -1)
            {
                Sprite_Draw(UI_Current_Weapon, Cur_Weapon_X, Cur_Weapon_Y, Cur_Weapon_Size, Cur_Weapon_Size);

                // Ammo Numbers (000 / 000)
                const auto& C_Weapon = P_Inventory->at(0);
                char AmmoBuffer[32];
                sprintf_s(AmmoBuffer, "%03d / %03d", C_Weapon.CurrentAmmo, C_Weapon.ReserveAmmo);

                // Draw Numbers inside Ammo Box
                float Num_W = Ammo_Box_W / 9.0f;
                float Num_H = Ammo_Box_H * 0.6f;
                float Num_Y = Ammo_Box_Y + (Ammo_Box_H - Num_H) * 0.5f; // Center Y

                // Get Middle X POS
                float Box_Center_X = Ammo_Box_X + (Ammo_Box_W * 0.5f);

                // Get Total_Text_Width
                // (Count) * (Wifth) * (0.8)
                float Total_Text_Width = 9.0f * (Num_W * 0.8f);

                // Get Start POS.X = Middle - (Hlaf Of Total Text Width) + (Little Bit)
                float Start_X = Box_Center_X - (Total_Text_Width * 0.5f);

                Draw_Number_String(AmmoBuffer, Start_X, Num_Y, Num_W, Num_H);
            }

            // B. Next Weapon 1 (Index 1)
            if (UI_Next_Weapon_1 != -1)
            {
                Sprite_Draw(UI_Next_Weapon_1, Next_Weapon_1_X, Next_Weapon_1_Y, Next_Weapon_Size, Next_Weapon_Size);
            }

            // C. Next Weapon 2 (Index 2)
            if (UI_Next_Weapon_2 != -1)
            {
                Sprite_Draw(UI_Next_Weapon_2, Next_Weapon_2_X, Next_Weapon_2_Y, Next_Weapon_Size, Next_Weapon_Size);
            }
        }
    }

    // -----------------------------------------------------------
    // 4. Reload
    // -----------------------------------------------------------
    bool isReloading = Weapon_System::GetInstance().Is_Reloading();
    bool isSwitching = Weapon_System::GetInstance().Is_Switching();

    if (isReloading || isSwitching)
    {
        bool IsGrenade = false;
        if (Weapon_System::GetInstance().HasWeapon())
        {
            if (Weapon_System::GetInstance().GetCurrentWeapon().Spec.Type == WeaponType::GRENADE)
                IsGrenade = true;
        }

        if (!IsGrenade || isSwitching)
        {
            Sprite_Draw(UI_Reload, Aim_X, Aim_Y, Aim_Size, Aim_Size, Reload_Rotation);
        }
    }
    // -----------------------------------------------------------
    // 5. Aim
    // -----------------------------------------------------------
    else
    {
        if (Game_UI_Aiming_Now())
        {
            Sprite_Draw(UI_Zoom_Aim, Aim_X, Aim_Y, Aim_Size, Aim_Size, Aim_Radian, Alpha_T_Quarter);
        }
        else
        {
            Sprite_Draw(UI_Normal_Aim, Aim_X, Aim_Y, Aim_Size, Aim_Size, Aim_Radian, Alpha_T_Quarter);
        }
    }

    // -----------------------------------------------------------
    // 6. Weapon Pick Up Prompt
    // -----------------------------------------------------------
    if (UI_Prompt != -1 && Get_Ready_Prompt_UI())
    {
        Sprite_Draw(UI_Prompt, Pick_X, Pick_Y, Pick_W, Pick_H);
    }

    // -----------------------------------------------------------
    // 7. Damager Overlay
    // -----------------------------------------------------------
    if (Is_Damage_Effect_On && Damage_Alpha > 0.0f)
    {
        float Screen_W = static_cast<float>(Direct3D_GetBackBufferWidth());
        float Screen_H = static_cast<float>(Direct3D_GetBackBufferHeight());

        Palette RedColor = { 1.0f, 0.0f, 0.0f, Damage_Alpha };
        Sprite_Draw(UI_Damage_Red, 0.0f, 0.0f, Screen_W, Screen_H, 0.0f, RedColor);
    }
}

void Game_UI_Texture()
{
	//----------------BG Texture----------------//
    UI_Damage_Red = Texture_Manager::GetInstance()->GetID("R");

	//----------------Aim Texture----------------//
    UI_Normal_Aim = Texture_Manager::GetInstance()->GetID("UI_Aim_Normal");
    UI_Zoom_Aim = Texture_Manager::GetInstance()->GetID("UI_Aim_Zomm");

	//----------------Bar Texture----------------//
	UI_HP_Bar   = Texture_Manager::GetInstance()->GetID("HP_Bar");
	UI_HP_Fill  = Texture_Manager::GetInstance()->GetID("HP_Fill");
	UI_EXP_Bar  = Texture_Manager::GetInstance()->GetID("EXP_Bar");
	UI_EXP_Fill = Texture_Manager::GetInstance()->GetID("EXP_Fill");

	//----------------Weapon Texture----------------//
    UI_Weapon_BG = Texture_Manager::GetInstance()->GetID("UI_Weapon_Box");
    UI_Ammo_BG = Texture_Manager::GetInstance()->GetID("UI_Ammo_Box");
    UI_Icon_HG = Texture_Manager::GetInstance()->GetID("UI_HG");
    UI_Icon_AR = Texture_Manager::GetInstance()->GetID("UI_AR");
    UI_Icon_MG = Texture_Manager::GetInstance()->GetID("UI_MG");
    UI_Icon_G  = Texture_Manager::GetInstance()->GetID("UI_Grenade");
    UI_Reload = Texture_Manager::GetInstance()->GetID("Reload");

	//----------------Numbers Texture----------------//
    UI_Num[0] = Texture_Manager::GetInstance()->GetID("UI_Num_MIN");
    UI_Num[1] = Texture_Manager::GetInstance()->GetID("UI_Num_1");
    UI_Num[2] = Texture_Manager::GetInstance()->GetID("UI_Num_2");
    UI_Num[3] = Texture_Manager::GetInstance()->GetID("UI_Num_3");
    UI_Num[4] = Texture_Manager::GetInstance()->GetID("UI_Num_4");
    UI_Num[5] = Texture_Manager::GetInstance()->GetID("UI_Num_5");
    UI_Num[6] = Texture_Manager::GetInstance()->GetID("UI_Num_6");
    UI_Num[7] = Texture_Manager::GetInstance()->GetID("UI_Num_7");
    UI_Num[8] = Texture_Manager::GetInstance()->GetID("UI_Num_8");
    UI_Num[9] = Texture_Manager::GetInstance()->GetID("UI_Num_9");
    UI_Num_Slash  = Texture_Manager::GetInstance()->GetID("UI_Num_Slash");
    UI_Percentage = Texture_Manager::GetInstance()->GetID("UI_Percentage");
}

void Game_UI_Trigger_Damage()
{
    Is_Damage_Effect_On = true;
    Damage_Effect_Timer = 0.0f;
}

void Game_UI_Is_Aiming_Mode(bool Is_Aiming)
{
    Aiming_Now = Is_Aiming;
}

bool Game_UI_Aiming_Now()
{
    return Aiming_Now;
}

void Draw_Number_String(const std::string& str, float startX, float startY, float width, float height)
{
    float Current_X = startX;

    for (char Num : str)
    {
        if (Num >= '0' && Num <= '9')
        {
            int Digit = Num - '0';
            if (UI_Num[Digit] != -1)
            {
                Sprite_Draw(UI_Num[Digit], Current_X, startY, width, height);
            }
        }
        else if (Num == '/')
        {
            Sprite_Draw(UI_Num_Slash, Current_X, startY, width, height);
        }
		else if (Num == '%')
		{
			Sprite_Draw(UI_Percentage, Current_X, startY, width, height);
		}

        Current_X += width * 0.8f;
    }
}

static int Get_Weapon_Icon_ID(WeaponType type)
{
    switch (type)
    {
    case WeaponType::HANDGUN:      
        return UI_Icon_HG;

    case WeaponType::ASSAULT_RIFLE: 
        return UI_Icon_AR;

    case WeaponType::MACHINE_GUN:  
        return UI_Icon_MG;

    case WeaponType::GRENADE:     
        return UI_Icon_G;
    }
    return -1;
}

void Get_Prompt_Tex_UI()
{
    if (XKeyLogger_IsControllerInput())
    {
        UI_Prompt = Texture_Manager::GetInstance()->GetID("Pick_UP_Controller");
    }
    else
    {
        UI_Prompt = Texture_Manager::GetInstance()->GetID("Pick_UP_Keyboard");
    }
}

bool Get_Ready_Prompt_UI()
{
    DirectX::XMFLOAT3 Eye = Player_Camera_Get_Current_POS();
    DirectX::XMFLOAT3 Dir = Player_Camera_Get_Front();

    ResourceItem* Item = Resource_Manager::GetInstance().Get_Nearest_Weapon_In_View(Eye, Dir, 5.0f);

    if (Item != nullptr) return true;
    else return false;
}
