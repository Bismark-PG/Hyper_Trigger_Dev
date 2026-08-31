/*==============================================================================

    Manage Weapon Logic and Inventory [Weapon_System.h]

    Author : Choi HyungJoon

==============================================================================*/
#ifndef WEAPON_SYSTEM_H
#define WEAPON_SYSTEM_H
#include <DirectXMath.h>
#include <vector>
#include "Weapon_Info.h" 

class Weapon_System
{
public:
    static Weapon_System& GetInstance()
    {
        static Weapon_System instance;
        return instance;
    }

    void Init();
    void Update(double elapsed_time);
	void Clear();

    bool Fire(const DirectX::XMFLOAT3& visualPos, const DirectX::XMFLOAT3& logicalPos, const DirectX::XMFLOAT3& dir, int damage);
    void Reload();
    
    bool AddWeapon(WeaponType type, bool Sound = true);
	void DropCurrentWeapon();

	// Get Current Weapon Info
    bool HasWeapon() const;
    const WeaponState& GetCurrentWeapon() const;

	// Inventory Info
    const std::deque<WeaponState>& GetInventory() const;

    // Set BGM According to Weapon
	void SyncBGM();
	void SyncBGM_Game_Reset();

    // --- Level Bonus Operations ---
    void Add_Ammo_Bonus(WeaponType type, int magBonus, int reserveBonus);

    // --- Reloading System ---
    bool Is_Reloading() const { return m_IsReloading; }
    float Get_Reload_State() const;

    bool Is_Switching() const { return m_IsSwitching; }
    float Get_Switching_State() const;

private:
    Weapon_System();
    ~Weapon_System();
    Weapon_System(const Weapon_System&) = delete;
    Weapon_System& operator=(const Weapon_System&) = delete;

	void CheckAmmoAndSwitch(); // Check Ammo And Switch Weapon If Needed

	std::deque<WeaponState> m_WeaponQueue; // Max Size 3
    float                   m_CoolDownTimer = 0.0f;
    const size_t            m_MaxInventorySize = 3;

	// Ammo Bonus For Each Weapon Type
    int m_Current_Ammo_Bonus[4] = { 0, 0, 0, 0 };
    int m_Reserve_Ammo_Bonus[4] = { 0, 0, 0, 0 };

    bool  m_IsReloading = false;
    float m_ReloadTimer = 0.0f;
    float m_ReloadMaxTime = 0.0f;
    int   m_ReloadStage = 0; // 0: Start, 1: Middle/Loop, 2: End
    
    float m_AutoFireBuffer = 0.0f;

    bool  m_IsSwitching = false;
    float m_SwitchTimer = 0.0f;
    const float m_SwitchMaxTime = 1.0f; // Weapon Switch Timer

    void Ammo_Reload();
};

#endif // WEAPON_SYSTEM_H