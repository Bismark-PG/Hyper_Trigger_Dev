/*==============================================================================

    Management Texture Resource [Game_Texture.h]

    Author : Choi HyungJoon

==============================================================================*/
#include "Game_Texture.h"
#include "Texture_Manager.h"
#include <unordered_map>
#include <string>

Texture_Manager* Texture_M;

void Game_Texture_Initialize()
{
    Texture_M = Texture_Manager::GetInstance();
    Game_Texture_Load();
}

void Game_Texture_Finalize()
{
}

void Game_Texture_Load()
{
    //------------------------------------------------Debug Texture------------------------------------------------//
    Texture_M->Load("Debug_Text", L"Resource/Texture/Other/Console_ASCII_512.png");
    
    Texture_M->Load("W", L"Resource/Texture/BG/RGBCMYK/White.png");
    Texture_M->Load("R", L"Resource/Texture/BG/RGBCMYK/Red.png");
    Texture_M->Load("G", L"Resource/Texture/BG/RGBCMYK/Green.png");
    Texture_M->Load("B", L"Resource/Texture/BG/RGBCMYK/Blue.png");
    Texture_M->Load("C", L"Resource/Texture/BG/RGBCMYK/Cyan.png");
    Texture_M->Load("M", L"Resource/Texture/BG/RGBCMYK/Magenta.png");
    Texture_M->Load("Y", L"Resource/Texture/BG/RGBCMYK/Yellow.png");
    Texture_M->Load("K", L"Resource/Texture/BG/RGBCMYK/Black.png");

    //-------------------------------------------<< In Game Texture >>-------------------------------------------//
    //-----------------------Mash Field Texture-----------------------//
    Texture_M->Load("Mash", L"Resource/Texture/OBJ/Mash.png");
    Texture_M->Load("Floor", L"Resource/Texture/OBJ/Floor.png");

    //----------------------Effects Texture----------------------//
    Texture_M->Load("Player_Explosion", L"Resource/Texture/Animation/Explosion_Light_1.png");
    Texture_M->Load("Enemy_Explosion", L"Resource/Texture/Animation/Explosion_Light_Big_Fixed.png");
    Texture_M->Load("Enemy_Real_Explosion", L"Resource/Texture/Animation/Explosion.png");

    //----------------------Billborad Texture----------------------//
    Texture_M->Load("Target", L"Resource/Texture/Other/Target.png");
    Texture_M->Load("EXP", L"Resource/Texture/Other/EXP.png");
    Texture_M->Load("HP", L"Resource/Texture/Other/HP.png");
    Texture_M->Load("Bullet", L"Resource/Texture/Other/Bullet.png");
    Texture_M->Load("Object", L"Resource/Texture/Other/OBJ.png");

    //---------------------------------------------<< Card Texture >>---------------------------------------------//
    //-----------------------------Weapon Card Texture-----------------------------//
    Texture_M->Load("Card_HG_Ammo", L"Resource/Texture/UI/Card/Card_HG_Ammo_UP.png");
    Texture_M->Load("Card_HG_Lock", L"Resource/Texture/UI/Card/Card_HG_Lock.png");

    Texture_M->Load("Card_AR_Ammo", L"Resource/Texture/UI/Card/Card_AR_Ammo_UP.png");
    Texture_M->Load("Card_AR_Lock", L"Resource/Texture/UI/Card/Card_AR_Lock.png");

    Texture_M->Load("Card_MG_Ammo", L"Resource/Texture/UI/Card/Card_MG_Ammo_UP.png");
    Texture_M->Load("Card_MG_Lock", L"Resource/Texture/UI/Card/Card_MG_Lock.png");

    Texture_M->Load("Card_G_Ammo", L"Resource/Texture/UI/Card/Card_G_Ammo_UP.png");
    Texture_M->Load("Card_G_Lock", L"Resource/Texture/UI/Card/Card_G_Lock.png");

    //--------------------------Stat Upgrade Card Texture--------------------------//
    Texture_M->Load("Card_HP_UP", L"Resource/Texture/UI/Card/Card_HP_UP.png");

    Texture_M->Load("Card_ATK_UP_Small", L"Resource/Texture/UI/Card/Card_ATK_UP_S.png");
    Texture_M->Load("Card_ATK_UP_Big", L"Resource/Texture/UI/Card/Card_ATK_UP_B.png");

    Texture_M->Load("Card_EXP_UP_Small", L"Resource/Texture/UI/Card/Card_EXP_UP_S.png");
    Texture_M->Load("Card_EXP_UP_Big", L"Resource/Texture/UI/Card/Card_EXP_UP_B.png");

    Texture_M->Load("Card_Speed_UP", L"Resource/Texture/UI/Card/Card_Speed_UP.png");
    
    //----------------------------------------------<< BG Texture >>----------------------------------------------//
    Texture_M->Load("BG_Menu", L"Resource/Texture/BG/BG_Menu.png");
    Texture_M->Load("BG_Title", L"Resource/Texture/BG/BG_Title.png");

    //----------------------------------------------<< UI Texture >>----------------------------------------------//
    //---------------------UI : Mouse Texture---------------------//
    Texture_M->Load("UI_Mouse_Cursor", L"Resource/Texture/UI/UI_Mouse_Cursor.png");
    
    //---------------------UI : BackGround Texture---------------------//
    Texture_M->Load("UI_Background", L"Resource/Texture/UI/UI_Background.png");

    //---------------------UI : In Game Texture---------------------//
    Texture_M->Load("UI_Aim_Normal", L"Resource/Texture/UI/UI_Normal_Aim.png");
    Texture_M->Load("UI_Aim_Zomm", L"Resource/Texture/UI/UI_Zoom_Aim.png");

    Texture_M->Load("HP_Bar", L"Resource/Texture/UI/UI_HP.png");
    Texture_M->Load("HP_Fill", L"Resource/Texture/UI/UI_HP_BAR.png");
    Texture_M->Load("EXP_Bar", L"Resource/Texture/UI/UI_EXP_L.png");
    Texture_M->Load("EXP_Fill", L"Resource/Texture/UI/UI_EXP_BAR_L.png");

    //---------------------UI : Weapon Texture---------------------//
    Texture_M->Load("UI_Weapon_Box", L"Resource/Texture/UI/UI_Weapon.png");
    Texture_M->Load("UI_Ammo_Box", L"Resource/Texture/UI/UI_Long_Square.png");

    Texture_M->Load("UI_HG", L"Resource/Texture/UI/UI_Drop_HG.png");
    Texture_M->Load("UI_AR", L"Resource/Texture/UI/UI_Drop_AR.png");
    Texture_M->Load("UI_MG", L"Resource/Texture/UI/UI_Drop_MG.png");
    Texture_M->Load("UI_Grenade", L"Resource/Texture/UI/UI_Drop_G.png");

    Texture_M->Load("UI_Drop_Box", L"Resource/Texture/UI/UI_Drop_Box.png");
    Texture_M->Load("UI_Drop_Box_HG", L"Resource/Texture/UI/UI_Drop_HG.png");
    Texture_M->Load("UI_Drop_Box_AR", L"Resource/Texture/UI/UI_Drop_AR.png");
    Texture_M->Load("UI_Drop_Box_MG", L"Resource/Texture/UI/UI_Drop_MG.png");
    Texture_M->Load("UI_Drop_Box_G", L"Resource/Texture/UI/UI_Drop_G.png");
    
    Texture_M->Load("Reload", L"Resource/Texture/UI/UI_Reload.png");

    Texture_M->Load("Pick_UP_Keyboard", L"Resource/Texture/UI/UI_Pick_UP_K.png");
    Texture_M->Load("Pick_UP_Controller", L"Resource/Texture/UI/UI_Pick_UP_C.png");

    //---------------------UI : Main Menu Texture---------------------//
    Texture_M->Load("Start", L"Resource/Texture/UI/Menu_Start.png");
    Texture_M->Load("Settings", L"Resource/Texture/UI/Menu_Settings.png");
    Texture_M->Load("Exit", L"Resource/Texture/UI/Menu_Exit.png");
    Texture_M->Load("UI_Back", L"Resource/Texture/UI/Menu_Back.png");

    //---------------------UI : Game Menu Texture---------------------//
    Texture_M->Load("Wave", L"Resource/Texture/UI/UI_Wave.png");
    Texture_M->Load("Endless", L"Resource/Texture/UI/UI_Endless.png");
    Texture_M->Load("Main", L"Resource/Texture/UI/UI_Main.png");
    Texture_M->Load("Return", L"Resource/Texture/UI/UI_Return.png");

    //--------------------UI : Settuing Menu Texture--------------------//
    Texture_M->Load("UI_BGM", L"Resource/Texture/UI/Settings_BGM.png");
    Texture_M->Load("UI_SFX", L"Resource/Texture/UI/Settings_SFX.png");
    Texture_M->Load("UI_Sensitivity", L"Resource/Texture/UI/Settings_Sensitivity.png");
    Texture_M->Load("UI_Window_Mode", L"Resource/Texture/UI/Settings_W_Mode_Fixed.png");
    Texture_M->Load("UI_Full_Screen_Mode", L"Resource/Texture/UI/Settings_FS_Mode.png");
    Texture_M->Load("UI_Run_Toggle", L"Resource/Texture/UI/Settings_Run_Toggle.png");
    Texture_M->Load("UI_View", L"Resource/Texture/UI/Settings_View.png");

    Texture_M->Load("UI_Toggle_Check", L"Resource/Texture/UI/Settings_Toggle_Check.png");
    Texture_M->Load("UI_Toggle_L", L"Resource/Texture/UI/Settings_Toggle_L.png");
    Texture_M->Load("UI_Toggle_R", L"Resource/Texture/UI/Settings_Toggle_R.png");

    //-----------------------UI : Number Texture-----------------------//
    Texture_M->Load("UI_Num_MIN", L"Resource/Texture/UI/Num/UI_Num_Min.png");
    Texture_M->Load("UI_Num_1", L"Resource/Texture/UI/Num/UI_Num_1.png");
    Texture_M->Load("UI_Num_2", L"Resource/Texture/UI/Num/UI_Num_2.png");
    Texture_M->Load("UI_Num_3", L"Resource/Texture/UI/Num/UI_Num_3.png");
    Texture_M->Load("UI_Num_4", L"Resource/Texture/UI/Num/UI_Num_4.png");
    Texture_M->Load("UI_Num_5", L"Resource/Texture/UI/Num/UI_Num_5.png");
    Texture_M->Load("UI_Num_6", L"Resource/Texture/UI/Num/UI_Num_6.png");
    Texture_M->Load("UI_Num_7", L"Resource/Texture/UI/Num/UI_Num_7.png");
    Texture_M->Load("UI_Num_8", L"Resource/Texture/UI/Num/UI_Num_8.png");
    Texture_M->Load("UI_Num_9", L"Resource/Texture/UI/Num/UI_Num_9.png");
    Texture_M->Load("UI_Num_MAX", L"Resource/Texture/UI/Num/UI_Num_Max.png");
    Texture_M->Load("UI_Num_Slash", L"Resource/Texture/UI/Num/UI_Num_Slash.png");
    Texture_M->Load("UI_Percentage", L"Resource/Texture/UI/Num/UI_Percentage.png");

    Texture_M->Load("UI_Num_Button_L", L"Resource/Texture/UI/Num/UI_Button_L.png");
    Texture_M->Load("UI_Num_Button_R", L"Resource/Texture/UI/Num/UI_Button_R.png");
    Texture_M->Load("UI_Score_Number", L"Resource/Texture/UI/Num/UI_Num_Animation.png");

    //---------------------UI : Controller Texture---------------------//
    Texture_M->Load("UI_Controller_Input", L"Resource/Texture/Controller/Controller_Input_Alert_Fixed.png");
    Texture_M->Load("UI_Controller_Output", L"Resource/Texture/Controller/Controller_Output_Alert_Fixed.png");
    Texture_M->Load("UI_Controller_Button_Up", L"Resource/Texture/Controller/A_Pressed_UP.png");
    Texture_M->Load("UI_Controller_Button_Down", L"Resource/Texture/Controller/A_Pressed_Fixed.png");
    Texture_M->Load("UI_Controller_Enter_Up", L"Resource/Texture/Controller/Enter_Alpha.png");
    Texture_M->Load("UI_Controller_Enter_Down", L"Resource/Texture/Controller/Enter_Alpha_Press.png");


    //-------------------------------------------------Others Texture-------------------------------------------------//
    Texture_M->Load("Bismark_LOGO", L"Resource/Texture/Other/Bismark.jpg");
    Texture_M->Load("Bismark_LOGO_B", L"Resource/Texture/Other/Bismark_Black.jpg");
    Texture_M->Load("Bismark_Name", L"Resource/Texture/Other/Name_W.png");
    Texture_M->Load("Pixel_Withe", L"Resource/Texture/Other/W_Pixel.png");
    Texture_M->Load("Pixel_Black", L"Resource/Texture/Other/B_Pixel.png");

    // Sample
    Texture_M->Load("TextSample", L"Resource/Texture/BG/RGBCMYK/White.png");
}