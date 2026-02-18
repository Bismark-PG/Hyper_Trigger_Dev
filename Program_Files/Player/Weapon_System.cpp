/*==============================================================================

    Manage Weapon Logic and Inventory [Weapon_System.cpp]

    Author : Choi HyungJoon

==============================================================================*/
#include "Weapon_System.h"
#include "Bullet_Manager.h"
#include "Audio_Manager.h"
#include "BGM_Mixer.h"
#include "KeyLogger.h" 

void Weapon_System::Init()
{
    Clear();
}

void Weapon_System::Update(double elapsed_time)
{
    float dt = static_cast<float>(elapsed_time);

    // -----------------------------------------------------------
    //                  Weapon Switching Logic
    // -----------------------------------------------------------
    if (m_IsSwitching)
    {
        m_SwitchTimer -= dt;

        if (m_SwitchTimer <= 0.0f)
        {
            m_IsSwitching = false;

            if (!m_WeaponQueue.empty())
            {
                m_WeaponQueue.pop_front();
            }

            Audio_Manager::GetInstance()->Play_SFX("Weapon_Change");
            SyncBGM();
        }

        return;
    }

    // -----------------------------------------------------------
    //                    Auto Fire Loop Logic
    // -----------------------------------------------------------
    if (m_AutoFireBuffer > 0.0f)
    {
        m_AutoFireBuffer -= dt;

        if (m_AutoFireBuffer <= 0.0f)
        {
            m_AutoFireBuffer = 0.0f;

            if (HasWeapon())
            {
                auto& weapon = GetCurrentWeapon();
                if (weapon.Spec.IsAutomatic)
                {
                    std::string LoopName = (weapon.Spec.Type == WeaponType::MACHINE_GUN) ? "MG_Fire" : "AR_Fire";
                    std::string EndName = (weapon.Spec.Type == WeaponType::MACHINE_GUN) ? "MG_Fire_End" : "AR_Fire_End";

                    if (Audio_Manager::GetInstance()->Is_Playing_Loop_SFX(LoopName))
                    {
                        Audio_Manager::GetInstance()->Stop_Loop_SFX(LoopName);
                        Audio_Manager::GetInstance()->Play_SFX(EndName);
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------
    //                       CoolDown Logic
    // -----------------------------------------------------------
    if (m_CoolDownTimer > 0.0f)
    {
        m_CoolDownTimer -= dt;
    }

    // -----------------------------------------------------------
    //                     Reload Process Logic
    // -----------------------------------------------------------
    if(m_IsReloading)
    {
        auto& weapon = GetCurrentWeapon();
        float TotalTime = weapon.Spec.ReloadTime;
        float Elapsed = TotalTime - m_ReloadTimer;

        // --- AR Logic (3.0s) ---
        // Start(0s) -> Ing(0.5s) -> Done(2.8s) -> Finish(3.0s)
        if (weapon.Spec.Type == WeaponType::ASSAULT_RIFLE)
        {
            if (m_ReloadStage == 0 && Elapsed >= 0.5f)
            {
                Audio_Manager::GetInstance()->Play_SFX("AR_Reload_Ing");
                m_ReloadStage = 1;
            }
            if (m_ReloadStage == 1 && m_ReloadTimer <= 0.2f)
            {
                Audio_Manager::GetInstance()->Play_SFX("AR_Reload_Done");
                m_ReloadStage = 2;
            }
        }

        // --- MG Logic (5.0s) ---
        // Start(0s) -> Loop Start(1.0s) -> Stop Loop & End(5.0s)
        else if (weapon.Spec.Type == WeaponType::MACHINE_GUN)
        {
            if (m_ReloadStage == 0 && Elapsed >= 1.0f)
            {
                Audio_Manager::GetInstance()->Play_Loop_SFX("MG_Reload_Ing");
                m_ReloadStage = 1;
            }
        }

        m_ReloadTimer -= dt;

        if (m_ReloadTimer <= 0.0f)
        {
            if (weapon.Spec.Type == WeaponType::MACHINE_GUN)
            {
                Audio_Manager::GetInstance()->Stop_Loop_SFX("MG_Reload_Ing");
                Audio_Manager::GetInstance()->Play_SFX("MG_Reload_End");
            }
            Ammo_Reload();
        }
    }
}

void Weapon_System::Clear()
{
    m_WeaponQueue.clear();
    m_CoolDownTimer = 0.0f;
    m_AutoFireBuffer = 0.0f;
    m_IsReloading = false;

    Audio_Manager::GetInstance()->Stop_Loop_SFX("AR_Fire");
    Audio_Manager::GetInstance()->Stop_Loop_SFX("MG_Fire");
    Audio_Manager::GetInstance()->Stop_Loop_SFX("MG_Reload_Ing");

    AddWeapon(WeaponType::HANDGUN, false);

    for (int i = 0; i < 4; i++)
    {
        m_Current_Ammo_Bonus[i] = 0;
        m_Reserve_Ammo_Bonus[i] = 0;
    }
}

bool Weapon_System::Fire(const DirectX::XMFLOAT3& visualPos, const DirectX::XMFLOAT3& logicalPos, const DirectX::XMFLOAT3& dir, int damage)
{
    if (m_WeaponQueue.empty()) return false;
    if (m_CoolDownTimer > 0.0f) return false;
    if (m_IsReloading) return false;

    WeaponState& weapon = m_WeaponQueue.front();

    if (weapon.CurrentAmmo <= 0)
    {
        if (weapon.ReserveAmmo > 0)
        {
            Reload();
            return false;
        }
        else
        {
            // Need Empty Sound?
            return false;
        }
    }

    // -------------------------------------------------------
    //                  Fire Process
    // -------------------------------------------------------
    m_CoolDownTimer = weapon.Spec.FireRate;
    weapon.CurrentAmmo--;

    if (weapon.Spec.IsAutomatic)
    {
        std::string LoopName = (weapon.Spec.Type == WeaponType::MACHINE_GUN) ? "MG_Fire" : "AR_Fire";
        Audio_Manager::GetInstance()->Play_Loop_SFX(LoopName);
        m_AutoFireBuffer = weapon.Spec.FireRate + 0.08f;
    }
    else
    {
        switch (weapon.Spec.Type)
        {
        case WeaponType::HANDGUN:
            Audio_Manager::GetInstance()->Play_SFX("HG_Fire");
            break;

        case WeaponType::GRENADE:
            Audio_Manager::GetInstance()->Play_SFX("Grenade_Drow");
            break;
        }
    }

    // Spawn Bullet
    Bullet_Manager::Instance().Fire(weapon.Spec.B_Type, visualPos, logicalPos, dir, damage, BulletOwner::PLAYER);

    // Check Ammo Switch
    CheckAmmoAndSwitch();

    return true;
}

void Weapon_System::Reload()
{
    if (m_IsReloading || !HasWeapon()) return;

    auto& weapon = GetCurrentWeapon();

    if (weapon.CurrentAmmo >= weapon.Spec.MaxMagazine) return; // Full Ammo
    if (weapon.ReserveAmmo <= 0) return; // No Ammo

    // Start Reload
    m_IsReloading = true;
    m_ReloadTimer = weapon.Spec.ReloadTime;
    m_ReloadStage = 0; // Reset Stage

    m_AutoFireBuffer = 0.0f;
    Audio_Manager::GetInstance()->Stop_Loop_SFX("AR_Fire");
    Audio_Manager::GetInstance()->Stop_Loop_SFX("MG_Fire");

    // [Start Sound Play]
    switch (weapon.Spec.Type)
    {
    case WeaponType::HANDGUN:
        Audio_Manager::GetInstance()->Play_SFX("HG_Reload_Start");
        break;

    case WeaponType::ASSAULT_RIFLE:
        Audio_Manager::GetInstance()->Play_SFX("AR_Reload_Start");
        break;

    case WeaponType::MACHINE_GUN:
        Audio_Manager::GetInstance()->Play_SFX("MG_Reload_Start");
        break;
    }
}

bool Weapon_System::AddWeapon(WeaponType type, bool Sound)
{
    if (m_WeaponQueue.size() >= m_MaxInventorySize)
    {
        return false;
    }

    WeaponState newState;
    switch (type)
    {
    case WeaponType::HANDGUN: 
        newState.Spec = SPEC_HANDGUN;
        break;

    case WeaponType::ASSAULT_RIFLE:
        newState.Spec = SPEC_RIFLE; 
        break;

    case WeaponType::MACHINE_GUN:  
        newState.Spec = SPEC_MACHINEGUN; 
        break;

    case WeaponType::GRENADE:     
        newState.Spec = SPEC_GRENADE;
        break;
    }

    int Index = static_cast<int>(type);
    int C_Bonus = m_Current_Ammo_Bonus[Index];
    int R_Bonus = m_Reserve_Ammo_Bonus[Index];

    newState.CurrentAmmo = newState.Spec.MaxMagazine + C_Bonus;
    newState.ReserveAmmo = newState.Spec.MaxReserve  + R_Bonus;

    m_WeaponQueue.push_back(newState);

    if (Sound)
    {
        Audio_Manager::GetInstance()->Play_SFX("Weapon_Pick_UP");
    }

    SyncBGM();

    return true;
}

bool Weapon_System::HasWeapon() const
{
    return !m_WeaponQueue.empty();
}

const WeaponState& Weapon_System::GetCurrentWeapon() const
{
    return m_WeaponQueue.front();
}

const std::deque<WeaponState>& Weapon_System::GetInventory() const
{
    return m_WeaponQueue;
}

float Weapon_System::Get_Reload_State() const
{
    if (!m_IsReloading || m_ReloadMaxTime <= 0.0f)
        return 0.0f;

    // 0.0 (Start) -> 1.0 (Done)
    float Progress = 1.0f - (m_ReloadTimer / m_ReloadMaxTime);

    // Safety Clamping (0 ~ 1)
    if (Progress < 0.0f)
    {
        Progress = 0.0f;
    }

    if (Progress > 1.0f)
    {
        Progress = 1.0f;
    }

    return Progress;
}

float Weapon_System::Get_Switching_State() const
{
    if (!m_IsSwitching || m_SwitchTimer <= 0.0f)
        return 0.0f;

    // 0.0 (Start) -> 1.0 (Done)
    float Progress = 1.0f - (m_SwitchTimer / m_SwitchMaxTime);

    // Safety Clamping (0 ~ 1)
    if (Progress < 0.0f)
    {
        Progress = 0.0f;
    }

    if (Progress > 1.0f) 
    {
        Progress = 1.0f;
    }

    return Progress;
}

// ------------------------------------------------------------
//                Weapon_System Private Logic
// ------------------------------------------------------------
Weapon_System::Weapon_System()
{
	Clear();
}

Weapon_System::~Weapon_System()
{
}

void Weapon_System::CheckAmmoAndSwitch()
{
    if (m_WeaponQueue.empty()) return;

    WeaponState& weapon = m_WeaponQueue.front();

    if (weapon.CurrentAmmo <= 0 && weapon.ReserveAmmo <= 0)
    {
        if (m_IsSwitching) return;

        Audio_Manager::GetInstance()->Stop_Loop_SFX("AR_Fire");
        Audio_Manager::GetInstance()->Stop_Loop_SFX("MG_Fire");

        if (weapon.Spec.Type == WeaponType::ASSAULT_RIFLE)
        {
            Audio_Manager::GetInstance()->Play_SFX("AR_Fire_End");
        }
        if (weapon.Spec.Type == WeaponType::MACHINE_GUN)
        {
            Audio_Manager::GetInstance()->Play_SFX("MG_Fire_End");
        }

        // Need Ammo Empty Sound?

        m_AutoFireBuffer = 0.0f;

        // Switch Timer Start
        m_IsSwitching = true;
        m_SwitchTimer = m_SwitchMaxTime;
    }
}

void Weapon_System::Ammo_Reload()
{
    m_IsReloading = false;
    m_ReloadTimer = 0.0f;

    if (m_WeaponQueue.empty()) return;

    // Get Weapon
    WeaponState& weapon = m_WeaponQueue.front();

    // Get Need Ammo ( Max - Now)
    int Ammo_Need = weapon.Spec.MaxMagazine - weapon.CurrentAmmo;
    int Ammo_Amount = (weapon.ReserveAmmo >= Ammo_Need) ? Ammo_Need : weapon.ReserveAmmo;

    weapon.CurrentAmmo += Ammo_Amount;
    weapon.ReserveAmmo -= Ammo_Amount;

    // AR And MG Sound Will Be Play In Update 
    switch (weapon.Spec.Type)
    {
    case WeaponType::HANDGUN:
        Audio_Manager::GetInstance()->Play_SFX("HG_Reload_Done");
        break;

    case WeaponType::GRENADE:
        Audio_Manager::GetInstance()->Play_SFX("Grenade_Next");
        break;
    }
}

void Weapon_System::SyncBGM()
{
    // 1. Weapon Layer Flags
    bool active_flags[static_cast<int>(InstrumentType::SYNTH_2) + 1] = { false };

    // 2. Check Current Weapon Queue
    for (const auto& weapon : m_WeaponQueue)
    {
        switch (weapon.Spec.InstrumentName)
        {
		case InstrumentType::KICK:
			active_flags[static_cast<int>(InstrumentType::KICK)] = true;
			break;
		case InstrumentType::GUITAR:
			active_flags[static_cast<int>(InstrumentType::GUITAR)] = true;
			break;
		case InstrumentType::SYNTH_1:
			active_flags[static_cast<int>(InstrumentType::SYNTH_1)] = true;
			break;
		case InstrumentType::SYNTH_2:
			active_flags[static_cast<int>(InstrumentType::SYNTH_2)] = true;
			break;
        }
    }

    // 3. Set Mixer Control Layer
	// Will Be Game Start, Snare Layer On 
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::SNARE), true);

	// Weapon Layers Set
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::KICK),    active_flags[static_cast<int>(InstrumentType::KICK)]);
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::GUITAR),  active_flags[static_cast<int>(InstrumentType::GUITAR)]);
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::SYNTH_1), active_flags[static_cast<int>(InstrumentType::SYNTH_1)]);
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::SYNTH_2), active_flags[static_cast<int>(InstrumentType::SYNTH_2)]);
}

void Weapon_System::SyncBGM_Game_Reset()
{
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::SNARE),   true);
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::KICK),    false);
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::GUITAR),  false);
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::SYNTH_1), false);
    Mixer_Control_Layer(Get_Instrument_Key(InstrumentType::SYNTH_2), false);
}

void Weapon_System::Add_Ammo_Bonus(WeaponType type, int magBonus, int reserveBonus)
{
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < 4)
    {
        m_Current_Ammo_Bonus[idx] += magBonus;
        m_Reserve_Ammo_Bonus[idx] += reserveBonus;
    }
}