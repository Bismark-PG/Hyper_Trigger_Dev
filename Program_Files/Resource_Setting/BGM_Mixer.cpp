/*==============================================================================

	Manage BGM [BGM_Mixer.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "BGM_Mixer.h"
#include "Audio_Manager.h"
#include "Setting.h"
#include <vector>

static bool Is_First_Entry = true;
constexpr float Vol_OFF = 0.0f;
static float Fade_Amount = 0.25f;

void Mixer_Init()
{
    // Set Mixer Info
    Is_First_Entry = true;

    // Load BGM
    std::vector<std::string> BGM_Parts = { "Bass", "Guitar", "Kick", "Snare", "Synth_1", "Synth_2" };
    Audio_Manager::GetInstance()->Play_Layered_BGM(BGM_Parts, true);

    // Set Volume
    Audio_Manager::GetInstance()->Set_Layer_Volume("Bass",    1.0f);

    Audio_Manager::GetInstance()->Set_Layer_Volume("Kick",    0.0f);
    Audio_Manager::GetInstance()->Set_Layer_Volume("Snare",   0.0f);
    Audio_Manager::GetInstance()->Set_Layer_Volume("Guitar",  0.0f);
    Audio_Manager::GetInstance()->Set_Layer_Volume("Synth_1", 0.0f);
    Audio_Manager::GetInstance()->Set_Layer_Volume("Synth_2", 0.0f);

    Audio_Manager::GetInstance()->Set_Target_BGM_Volume(0.0f);
}

void Mixer_First_Game_Start(double elapsed_time)
{
    if (!Is_First_Entry)
    {
        return;
    }

    int User_Setting_Level = static_cast<int>(Get_BGM_Scale_Buffer());

    float Final_Target_Vol = static_cast<float>(User_Setting_Level) / 10.0f;

    float CurrentVol = Audio_Manager::GetInstance()->Get_Target_BGM_Volume();

    if (CurrentVol < Final_Target_Vol)
    {
        CurrentVol += static_cast<float>(elapsed_time) * Fade_Amount;

        if (CurrentVol > Final_Target_Vol)
            CurrentVol = Final_Target_Vol;

        Audio_Manager::GetInstance()->Set_Target_BGM_Volume(CurrentVol);
    }
    else
    {
        Is_First_Entry = false;
    }
}

void Mixer_Control_Layer(const char* LayerName, bool IsTurnOn)
{
    std::string name = LayerName;
    float volume = IsTurnOn ? 1.0f : Vol_OFF;

    if (name == "END_SIGNAL") return;

    Audio_Manager::GetInstance()->Set_Layer_Volume(name, volume);
}