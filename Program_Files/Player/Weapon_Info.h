/*==============================================================================

    Manage Weapon Info [Weapon_Info.h]

    Author : Choi HyungJoon

==============================================================================*/
#ifndef WEAPON_INFO_H
#define WEAPON_INFO_H
#include "Bullet.h" 
#include <string>
#include <deque>   

enum class InstrumentType
{
    NONE,
	KICK,
	SNARE,
	GUITAR,
	SYNTH_1,
	SYNTH_2
};

enum class WeaponType
{
    HANDGUN,
    ASSAULT_RIFLE,
    MACHINE_GUN,
    GRENADE
};

struct WeaponSpec
{
    WeaponType      Type = WeaponType::HANDGUN;
    BulletType      B_Type = BulletType::RAY;
    float           FireRate = 0.0f;
    bool            IsAutomatic = false;
    int             Damage = 0;

    // Ammo Specs
    int             MaxMagazine = 0; // Ammo per Magazine
    int             MaxReserve = 0; // Ammo in Reserve
    float           ReloadTime = 0.0f;

    // DJing Info
	InstrumentType  InstrumentName = InstrumentType::NONE;
};

struct WeaponState
{
    WeaponSpec  Spec = {};
    int         CurrentAmmo = 0;
    int         ReserveAmmo = 0;
};

// --- Weapon Specifications ---

// 1. Handgun (Kick) - 12 / 36 / 2 Sec
static const WeaponSpec SPEC_HANDGUN =
{
    WeaponType::HANDGUN, BulletType::RAY, 0.1f, false, 20,
    12, 36, 2.0f,
    InstrumentType::KICK
};

// 2. Assault Rifle (Synth_1) - 30 / 90 / 3 Sec
static const WeaponSpec SPEC_RIFLE =
{
    WeaponType::ASSAULT_RIFLE, BulletType::RAY, 0.1f, true, 15,
    30, 90, 3.0f,
    InstrumentType::SYNTH_1
};

// 3. Machine Gun (Guitar) - 120 / 240 / 5 Sec
static const WeaponSpec SPEC_MACHINEGUN =
{
    WeaponType::MACHINE_GUN, BulletType::RAY, 0.06f, true, 10,
    120, 240, 5.0f,
	InstrumentType::GUITAR
};

// 4. Grenade (Synth_2) - 1 / 6 / 0 Sec
static const WeaponSpec SPEC_GRENADE =
{
    WeaponType::GRENADE, BulletType::GRENADE, 1.0f, false, 50,
    1, 6, 0.5f,
	InstrumentType::SYNTH_2
};

// Get Instrument Key For BGM Sync
inline const char* Get_Instrument_Key(InstrumentType type)
{
    switch (type)
    {
    case InstrumentType::KICK:      return "Kick";
    case InstrumentType::SNARE:     return "Snare";
    case InstrumentType::GUITAR:    return "Guitar";
    case InstrumentType::SYNTH_1:   return "Synth_1";
    case InstrumentType::SYNTH_2:   return "Synth_2";
    default:                        return "";
    }
}

#endif // WEAPON_INFO_H  