/*==============================================================================

	Manage Player Logic [Player.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "Player.h"
#include "KeyLogger.h"
#include "model.h"
#include "Shader_Manager.h"
#include "Player_Camera.h" 
#include "Game_Model.h"
#include "Debug_Collision.h"
#include "Palette.h"
#include "Map_System.h"
#include "Game.h"
#include "Mash_Field.h"
#include "Weapon_System.h"
#include "Resource_Manager.h"
#include "Game_UI.h"
#include "Fade.h"   
#include "Game_Screen_Manager.h" 
#include "BGM_Mixer.h"
#include "Audio_Manager.h"
#include "Billboard_Manager.h"
#include "Setting.h"
using namespace DirectX;
using namespace PALETTE;

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30

// Player Physics Constants
constexpr float PLAYER_SCALE = 0.01f;
constexpr float PLAYER_DRAG = 5.0f;

static XMFLOAT3 Player_Pos	 = {};
static XMFLOAT3 Player_Start_POS = {};
static XMFLOAT3 Player_Front = { 0.0f, 0.0f, 1.0f };
static XMFLOAT3 Player_Vel	 = {};
static XMVECTOR Player_Speed = {};

static XMVECTOR Gravity		= { 0.0f, -15.0f, 0.0f };
static XMVECTOR Walk_Speed	= { 35.f, 0.0, 35.f };
static XMVECTOR Run_Speed	= { 75.f, 0.0, 75.f };

static MODEL* Player_Model{nullptr};
static bool Is_Input_Moving = false;
static bool Is_Jump = false;
static bool Is_Run = false;
static bool Is_Run_Toggle = false;

// Player resource
static const int Player_Base_MaxHP = 100;
static int Player_HP = 100;
static int Player_MaxHP = 100;
static float Player_Damage = 1.0f;

static bool Player_Is_Dead = false;
static bool Is_Shoot = false;
static bool Is_Aiming_Mode = false;
static bool Is_Game_Over_Sequence = false;

static bool  Is_Invincible = false;     
static float Invincible_Timer = 0.0f;   
constexpr float INVINCIBLE_DURATION = 1.0f;

static float Walk_Timer = 0.0f;

// Weapon Drop System
static float Interact_Timer = 0.0f;
static bool Has_Dropped_Weapon = false;
static bool Prev_Interact_Input = false;

// Level Bonus Ratios
static float Bonus_Damage_Ratio = 0.0f;
static float Bonus_Speed_Ratio = 0.0f;
static float Bonus_HP_Ratio = 0.0f;
constexpr float Bonus_HP_Heal = 0.1f;

static E_PlayerState Current_State = E_PlayerState::IDLE;

// ----------------------------------------------------------
//				static Player Update Logic
// ----------------------------------------------------------
// --- Movement And Physics System ---
static void Player_Update_Teleport_System();
static void Player_Update_Respawn_System(XMVECTOR Velocity);
static XMVECTOR Player_Update_AABB_System(XMVECTOR Velocity);
static void Player_Update_Bullet_System();

// --- Input ---
static XMVECTOR Player_Update_Is_Jumped(XMVECTOR Vel);
static XMVECTOR Player_Update_Is_Moved(XMVECTOR Dir, XMVECTOR Flat_Front, XMVECTOR Flat_Right);
static void Player_Update_Get_Sprint_Input();
static void Player_Update_Get_Drop_Item_Input(float dt);
static void Player_Update_Get_Reload_Input();
static void Player_Update_Get_Mouse_Input();

// --- Animation Model ---
static void Player_Update_Change_Animation(float dt);
// ----------------------------------------------------------

void Player_Initialize(const XMFLOAT3& First_POS, const XMFLOAT3& First_Front)
{
	Player_Pos = First_POS;
	Player_Start_POS = First_POS; 
	Player_Vel = { 0.0f, 0.0f, 0.0f };
	XMStoreFloat3(&Player_Front, XMVector3Normalize(XMLoadFloat3(&First_Front)));

	Player_Model = Model_Manager::GetInstance()->GetModel("Player");

	Is_Jump = false;
	Is_Run = false;
	Is_Run_Toggle = Setting_Get_Sprint_Type();

	Player_HP = Player_Base_MaxHP;
	Player_MaxHP = Player_Base_MaxHP;
	Player_Damage = 1.0f;

	Player_Is_Dead = false;
	Is_Shoot = false;
	Is_Aiming_Mode = false;
	Is_Game_Over_Sequence = false;

	Is_Invincible = false;
	Invincible_Timer = 0.0f;

	Weapon_System::GetInstance().Init();
}

void Player_Finalize()
{
	Player_Model = nullptr;
}

void Player_Update(double elapsed_time)
{
	float dt = static_cast<float>(elapsed_time);

	// ---------------------------------------------
	//	 --- 1. Game Over And Invincible Logic ---
	// ---------------------------------------------
	if (Player_Is_Dead || Is_Game_Over_Sequence) return;

	if (Is_Invincible)
	{
		Invincible_Timer -= dt;

		if (Invincible_Timer <= 0.0f)
		{
			Is_Invincible = false;
			Invincible_Timer = 0.0f;
		}
	}

	// ------------------------
	//	--- 2. Input Check ---
	// ------------------------
	// Get Mouse Input State For Check Fire
	Player_Update_Get_Mouse_Input();

	// Sprint (Run) Input
	Player_Update_Get_Sprint_Input();

	// Reload Input
	Player_Update_Get_Reload_Input();

	// Item Pickup Input
	Player_Update_Get_Drop_Item_Input(dt);

	// ---------------------------------
	//	--- 3. Physics Calculation ---
	// ---------------------------------
	XMVECTOR POS = XMLoadFloat3(&Player_Pos);
	XMVECTOR Vel = XMLoadFloat3(&Player_Vel);

	// Jump Logic
	Vel = Player_Update_Is_Jumped(Vel);

	// Gravity Apply
	Vel += Gravity * dt;

	// Movement Logic
	XMVECTOR Dir{};
	XMVECTOR Front = XMLoadFloat3(&Player_Camera_Get_Front());
	XMVECTOR Flat_Front = XMVector3Normalize(XMVectorSetY(Front, 0.0f));
	XMVECTOR Flat_Right = XMVector3Cross({ 0.0f,1.0f,0.0f }, Flat_Front);
	Is_Input_Moving = false;

	Dir = Player_Update_Is_Moved(Dir, Flat_Front, Flat_Right);
	Dir = XMVector3Normalize(Dir);

	if (Is_Player_Jump())
	{
		Dir = XMVectorZero();
	}

	// Set Direction And Speed
	Vel += Dir * Player_Speed * dt;

	// Animation Model Change
	Player_Update_Change_Animation(dt);

	// Friction

	XMVECTOR Horizontal_Vel = XMVectorMultiply(Vel, XMVectorSet(1.0f, 0.0f, 1.0f, 0.0f));
	if (!Is_Player_Jump())
	{
		Horizontal_Vel -= Horizontal_Vel * PLAYER_DRAG * dt;
	}
	else
	{
		Horizontal_Vel -= Horizontal_Vel * (PLAYER_DRAG * 0.05f) * dt;
	}
	Vel = XMVectorMultiply(Vel, XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)) + Horizontal_Vel;

	// Apply Velocity
	POS += Vel * dt;
	XMStoreFloat3(&Player_Pos, POS);

	// Reset Delta Time For Clamping
	if (dt > 0.05f)
	{
		dt = 0.05f;
	}

	// ------------------------------------------------------
	//	--- 4. Collision & Respawn (Physics Correction) ---
	// ------------------------------------------------------
	Player_Update_Respawn_System(Vel);
	Vel = Player_Update_AABB_System(Vel);
	Player_Update_Teleport_System();

	// ---------------------------
	//	--- 5. Weapon System ---
	// ---------------------------
	Weapon_System::GetInstance().Update(dt);

	// Camera Manage Logic
	Player_Camera_Set_Aiming_Mode(Player_Is_Aiming_Now());
	Game_UI_Is_Aiming_Mode(Player_Is_Aiming_Now());

	// Shooting Logic
	Player_Update_Bullet_System();

	// Save Final Front & Vel
	XMStoreFloat3(&Player_Front, Flat_Front);
	XMStoreFloat3(&Player_Vel, Vel);

	// Update Animation Time
	Player_Model_Animation_Update(dt);
}

void Player_Model_Animation_Update(float Time)
{
	Model_Update_Animation(Player_Model, Time);
}

void Player_Draw()
{
	if (!Player_Model) return;

	Shader_Manager::GetInstance()->SetLightSpecular(Player_Camera_Get_POS(), 164.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

	// Set Player Model Scale
	XMMATRIX S = XMMatrixScaling(PLAYER_SCALE, PLAYER_SCALE, PLAYER_SCALE);

	// Get Movement
	XMMATRIX T = XMMatrixTranslation(Player_Pos.x, Player_Pos.y, Player_Pos.z);

	// Get Yaw
	float Yaw = atan2f(Player_Front.x, Player_Front.z);
	float Model_Yaw = Yaw + XM_PI;
	XMMATRIX R = XMMatrixRotationY(Model_Yaw);

	// Reverse Player
	// Scale >> Rotate >> Translate
	XMMATRIX world = S * R * T;

	ModelDraw(Player_Model, world);

	if (Get_Debug_Mode_State())
	{
		Debug_Collision_Draw(Player_Get_AABB(), Red);
		ModelDraw_Bone(Player_Model, world);
	}
}

void Player_Draw_Shadow(const DirectX::XMMATRIX& LightViewProj)
{
	if (!Player_Model) return;

	// Set World Matrix (Must Be Same To Player_Draw POS)
	XMMATRIX S = XMMatrixScaling(PLAYER_SCALE, PLAYER_SCALE, PLAYER_SCALE);
	XMMATRIX T = XMMatrixTranslation(Player_Pos.x, Player_Pos.y, Player_Pos.z);

	// Set Rotation
	float Yaw = atan2f(Player_Front.x, Player_Front.z);
	float Model_Yaw = Yaw + XM_PI;
	XMMATRIX R = XMMatrixRotationY(Model_Yaw);

	// Set Final World Matrix
	// Matrix = Scale 8 Rotation * Translation
	XMMATRIX World = S * R * T;

	// Send Matrix To Shader Manager
	// World + LightViewProjection Matrix
	Shader_Manager::GetInstance()->SetShadowWorldMatrix(World, LightViewProj);

	// Drow Model Without Texture (Just Draw Shape)
	Shader_Manager::GetInstance()->DrawModelShadow_Animation(Player_Model, World, LightViewProj);
}

void Player_Reset()
{
	// 1. Movement Reset
	Player_Pos = Player_Start_POS;
	Player_Vel = { 0.0f, 0.0f, 0.0f };
	Player_Speed = { 0.0f, 0.0f, 0.0f, 0.0f };

	// 2. Resource Reset
	Player_HP = Player_Base_MaxHP;
	Player_MaxHP = Player_Base_MaxHP;
	Player_Damage = 1.0f;

	Player_Is_Dead = false;
	Is_Shoot = false;
	Is_Aiming_Mode = false;
	Is_Game_Over_Sequence = false;

	// 3. Invincible Reset
	Bonus_Damage_Ratio = 0.0f;
	Bonus_Speed_Ratio = 0.0f;
	Bonus_HP_Ratio = 0.0f;

	// 4. Drop Weapon Reset
	Interact_Timer = 0.0f;
	Has_Dropped_Weapon = false;
	Prev_Interact_Input = false;
}


void Player_Set_Sprint_Toggle_Mode(bool isToggle)
{
	Is_Run_Toggle = isToggle;
}

void Player_Set_POS(XMFLOAT3& POS)
{
	Player_Pos = POS;
}

const XMFLOAT3& Player_Get_POS()
{
	return Player_Pos;
}

const XMFLOAT3& Player_Get_Front()
{
	return Player_Front;
}

AABB Player_Get_AABB()
{
	float w = 0.25f;
	float h = 1.75f;
	float d = 0.25f;

	return AABB
	{
		{ Player_Pos.x + w, Player_Pos.y + h, Player_Pos.z + d }, // Max
		{ Player_Pos.x - w, Player_Pos.y	, Player_Pos.z - d }  // Min
	};
}

void Player_OnDamage(int damage)
{
	if (Player_Is_Dead || Is_Game_Over_Sequence) return;
	if (Is_Invincible) return;

	Audio_Manager::GetInstance()->Play_SFX("Player_Hit");

	Player_HP -= damage;

	Game_UI_Trigger_Damage();

	Is_Invincible = true;
	Invincible_Timer = INVINCIBLE_DURATION;

	if (Player_HP <= 0)
	{
		Player_Is_Dead = true;
		Is_Shoot = false;
		Is_Aiming_Mode = false;
		Is_Game_Over_Sequence = true;
	}
}

void Player_Heal(int amount)
{
	if (Player_Is_Dead) return;
	Player_HP += amount;
	if (Player_HP > Player_MaxHP) Player_HP = Player_MaxHP;
}

int Player_Get_HP()
{
	return Player_HP;
}

int Player_Get_MaxHP()
{
	return Player_MaxHP;
}

bool Player_Check_Is_Dead()
{
	return Is_Game_Over_Sequence;
}

bool Player_Is_Aiming_Now()
{
	return Is_Aiming_Mode;
}

void Player_LevelUp()
{
	// Heal
	// int healAmount = static_cast<int>(Player_Get_MaxHP() * Bonus_HP_Heal);
	int healAmount = static_cast<int>(Player_Get_MaxHP());
	Player_Heal(healAmount);

	Audio_Manager::GetInstance()->Play_SFX("Player_Level_UP");
	// Particle_Manager::GetInstance().Spawn_LevelUp_Effect(Player_Get_POS()); << Need?
}

float Player_Get_Damage_Coefficient()
{
	return Player_Damage * (1.0f + Bonus_Damage_Ratio);
}

void Player_Add_Damage_Bonus(float amount)
{
	Bonus_Damage_Ratio += amount;
}

void Player_Add_Speed_Bonus(float amount)
{
	Bonus_Speed_Ratio += amount;
}

void Player_Add_HP_Bonus(float amount)
{
	Bonus_HP_Ratio += amount;

	int addedHP = static_cast<int>(Player_Base_MaxHP * amount);

	Player_MaxHP += addedHP;
	Player_HP += addedHP;
}

// ----------------------------------------------------------------------------------------------------------------
//												Player Update Logic
//											  --- Movement System ---
// ----------------------------------------------------------------------------------------------------------------
static void Player_Update_Teleport_System()
{
	float Half_Size_X = Mash_Field_Get_Size_X() * 0.5f;
	float Half_Size_Z = Mash_Field_Get_Size_Z() * 0.5f;

	bool Teleport = false;

	if (Player_Pos.x > Half_Size_X)
	{
		Player_Pos.x -= Mash_Field_Get_Size_X();
		Teleport = true;
	}
	else if (Player_Pos.x < -Half_Size_X)
	{
		Player_Pos.x += Mash_Field_Get_Size_X();
		Teleport = true;
	}

	if (Player_Pos.z > Half_Size_Z)
	{
		Player_Pos.z -= Mash_Field_Get_Size_Z();
		Teleport = true;
	}
	else if (Player_Pos.z < -Half_Size_Z)
	{
		Player_Pos.z += Mash_Field_Get_Size_Z();
		Teleport = true;
	}
}

void Player_Update_Respawn_System(XMVECTOR Velocity)
{
	XMVECTOR Vel = Velocity;

	// Ground Collision Check
	float Ground_Y = Mash_Field_Get_Height(Player_Pos.x, Player_Pos.z);

	if (Ground_Y > -1000.0f)
	{
		if (Player_Pos.y < Ground_Y && XMVectorGetY(Vel) <= 0.0f)
		{
			Player_Pos.y = Ground_Y;
			Vel = XMVectorSetY(Vel, 0.0f);
			Player_Vel.y = 0.0f;

			Is_Jump = false;
		}
	}

	if (Player_Pos.y < -100.0f)
	{
		Player_Pos = Player_Start_POS;

		Player_Vel = { 0.0f, 0.0f, 0.0f };
	}
}

static XMVECTOR Player_Update_AABB_System(XMVECTOR Velocity)
{
	XMVECTOR Vel = Velocity;

	// Check Collision With Map Objects
	int OBJ_Count = Map_System_Get_Object_Count();
	float skinWidth = 0.05f;

	// Get Player AABB
	AABB Vertical_AABB = Player_Get_AABB();
	AABB Horizontal_AABB = Player_Get_AABB();

	// Get Flat Front
	Vertical_AABB.Min.x += skinWidth; Vertical_AABB.Max.x -= skinWidth;
	Vertical_AABB.Min.z += skinWidth; Vertical_AABB.Max.z -= skinWidth;

	for (int i = 0; i < OBJ_Count; i++)
	{
		const MapObject* OBJ = Map_System_Get_Object(i);
		if (OBJ->OBJ_ID == FIELD)
			continue;

		// Y Axis Collision
		IsHit Hit = Collision_Is_Hit_AABB(Vertical_AABB, OBJ->Collision);

		if (Hit.Is_Hit && Hit.Normal.y != 0.0f)
		{
			// Y Push Out
			XMVECTOR vNormal = XMLoadFloat3(&Hit.Normal);
			XMVECTOR vPush = vNormal * Hit.Depth;
			XMVECTOR vCurrentPos = XMLoadFloat3(&Player_Pos);
			vCurrentPos += vPush;
			XMStoreFloat3(&Player_Pos, vCurrentPos);

			// Landed on Object
			if (Hit.Normal.y > 0.5f)
			{
				Vel = XMVectorSetY(Vel, 0.0f);
				Player_Vel.y = 0.0f;
				Is_Jump = false;
			}
			// Hit Head
			else if (Hit.Normal.y < -0.5f)
			{
				Vel = XMVectorSetY(Vel, 0.0f);
				Player_Vel.y = 0.0f;
			}

			// Re-calc AABB
			Vertical_AABB = Player_Get_AABB();
			Vertical_AABB.Min.x += skinWidth; Vertical_AABB.Max.x -= skinWidth;
			Vertical_AABB.Min.z += skinWidth; Vertical_AABB.Max.z -= skinWidth;
		}
	}

	// XZ Axis Collision
	for (int i = 0; i < OBJ_Count; i++)
	{
		const MapObject* OBJ = Map_System_Get_Object(i);
		if (OBJ->OBJ_ID == FIELD)
			continue;

		IsHit Hit = Collision_Is_Hit_AABB(Horizontal_AABB, OBJ->Collision);

		// XZ Axis Collision
		if (Hit.Is_Hit && Hit.Normal.y == 0.0f)
		{
			// Push Out
			XMVECTOR vNormal = XMLoadFloat3(&Hit.Normal);
			XMVECTOR vPush = vNormal * Hit.Depth;
			XMVECTOR vCurrentPos = XMLoadFloat3(&Player_Pos);
			vCurrentPos += vPush;
			XMStoreFloat3(&Player_Pos, vCurrentPos);

			// Wall Sliding
			float velocityDot = XMVectorGetX(XMVector3Dot(Vel, vNormal));
			if (velocityDot < 0.0f)
			{
				Vel -= vNormal * velocityDot;
			}

			// AABB Update
			Horizontal_AABB = Player_Get_AABB();
		}
	}

	return Vel;
}

void Player_Update_Bullet_System()
{
	if (!Is_Player_Shoot()) return; // If Not Fired, Do Not Make Bullet

	// Logical Fire POS
	XMFLOAT3 Cam_Pos = Player_Camera_Get_Current_POS();
	XMFLOAT3 Cam_Dir = Player_Camera_Get_Front();

	// Virtual Fire POS
	XMFLOAT3 Player_Center = Player_Get_POS();

	// Default Damage
	int Base_Damage = Weapon_System::GetInstance().GetCurrentWeapon().Spec.Damage;

	// Bonus Damage
	float Bonus_Damage = Player_Get_Damage_Coefficient();

	// Final Damage
	int Final_Damage = static_cast<int>(Base_Damage * Bonus_Damage);

	// In Now Hard-Coeding Logic, After Need Player Fire POS Logic.
	XMVECTOR vP_Pos = XMLoadFloat3(&Player_Center);
	XMVECTOR vP_Front = XMLoadFloat3(&Player_Get_Front());
	XMVECTOR vUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR vRight = XMVector3Cross(vUp, vP_Front);
	XMVECTOR vGun_Pos = vP_Pos + (vUp * 1.5f) + (vRight * 0.5f) + (vP_Front * 0.5f);
	XMFLOAT3 Gun_Pos;
	XMStoreFloat3(&Gun_Pos, vGun_Pos);

	// Make Bullet, Fire!
	Weapon_System::GetInstance().Fire(Gun_Pos, Cam_Pos, Cam_Dir, Final_Damage);
}

// ----------------------------------------------------------------------------------------------------------------
//												--- Input ---
// ----------------------------------------------------------------------------------------------------------------
XMVECTOR Player_Update_Is_Jumped(XMVECTOR Vel)
{
	static BYTE Prev_LT = 0;
	BYTE Curr_LT = XKeyLogger_GetLeftTrigger();
	bool Is_LT_Triggered = (Curr_LT > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) && (Prev_LT <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	Prev_LT = Curr_LT;

	// Jump Logic
	if ((KeyLogger_IsTrigger(KK_SPACE) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_A) || Is_LT_Triggered) && !Is_Jump)
	{
		Is_Jump = true;
		Vel = XMVectorSetY(Vel, 0.0f);
		Vel += XMVECTOR{ 0.0f, 10.0f, 0.0f };
		// Audio_Manager::GetInstance()->Play_SFX("Player_Jump");
	}

	return Vel;
}

XMVECTOR Player_Update_Is_Moved(XMVECTOR Dir, XMVECTOR Flat_Front, XMVECTOR Flat_Right)
{
	// 1. Vector Init
	XMVECTOR InputDir = XMVectorZero();

	// 2-1. Get Vector From Keyboard Input
	if (KeyLogger_IsPressed(KK_W)) InputDir += Flat_Front;
	if (KeyLogger_IsPressed(KK_S)) InputDir -= Flat_Front;
	if (KeyLogger_IsPressed(KK_D)) InputDir += Flat_Right;
	if (KeyLogger_IsPressed(KK_A)) InputDir -= Flat_Right;

	// 2-2. Get Vector From Gamepad D-Pad Input
	if (XKeyLogger_IsPadPressed(XINPUT_GAMEPAD_DPAD_UP))    InputDir += Flat_Front;
	if (XKeyLogger_IsPadPressed(XINPUT_GAMEPAD_DPAD_DOWN))  InputDir -= Flat_Front;
	if (XKeyLogger_IsPadPressed(XINPUT_GAMEPAD_DPAD_RIGHT)) InputDir += Flat_Right;
	if (XKeyLogger_IsPadPressed(XINPUT_GAMEPAD_DPAD_LEFT))  InputDir -= Flat_Right;

	// 2-3. Get Vector From Gamepad Stick Input
	XMFLOAT2 Stick = XKeyLogger_GetLeftStick();
	float Stick_Magnitude = sqrtf(Stick.x * Stick.x + Stick.y * Stick.y); // Pythagorean Theorem

	if (Stick_Magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		// Normalize Stick Vector
		XMVECTOR StickDir = XMVectorSet(Stick.x, 0.0f, Stick.y, 0.0f);
		StickDir = XMVector3Normalize(StickDir);

		// Transform Stick Vector to World Space (Relative to Camera)
		// Stick.y (Up) -> Flat_Front
		// Stick.x (Right) -> Flat_Right
		XMVECTOR WorldStickDir = (Flat_Front * (Stick.y / 32767.0f)) + (Flat_Right * (Stick.x / 32767.0f));
		InputDir += WorldStickDir;
	}

	// 3. Get Length Squared
	XMVECTOR LengthSq = XMVector3LengthSq(InputDir);
	float lenSq = XMVectorGetX(LengthSq);

	// 4. If Moved, Normalize And Return
	if (lenSq > 0.0001f)
	{
		Is_Input_Moving = true;
		return Dir + XMVector3Normalize(InputDir);
	}
	else
	{
		Is_Input_Moving = false;
		return Dir;
	}
}

void Player_Update_Get_Sprint_Input()
{
	// Get Sprint Type From Setting
	Is_Run_Toggle = Setting_Get_Sprint_Type();

	// Get Sprint Input
	bool Is_Input_Sprint = KeyLogger_IsPressed(KK_LEFTSHIFT) || XKeyLogger_IsPadPressed(XINPUT_GAMEPAD_LEFT_THUMB) || XKeyLogger_IsPadPressed(XINPUT_GAMEPAD_RIGHT_SHOULDER);
	bool Is_Trigger_Sprint = KeyLogger_IsTrigger(KK_LEFTSHIFT) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_LEFT_THUMB) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_RIGHT_SHOULDER);

	if (Is_Run_Toggle)
	{
		// Toggle Mode
		if (Is_Trigger_Sprint)
			Is_Run = !Is_Run;
	}
	else
	{
		// Hold Mode
		Is_Run = Is_Input_Sprint;
	}

	if (Is_Run)
	{
		Player_Speed = Run_Speed * (A_Origin + Bonus_Speed_Ratio);
	}
	else
	{
		Player_Speed = Walk_Speed * (A_Origin + Bonus_Speed_Ratio);
	}
}

void Player_Update_Get_Drop_Item_Input(float dt)
{
	bool Current_Input = (KeyLogger_IsTrigger(KK_E) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_X));

	if (Current_Input)
	{
		if (!Has_Dropped_Weapon)
		{
			Interact_Timer += dt;

			if (Interact_Timer >= 2.0f)
			{
				if (Weapon_System::GetInstance().HasWeapon())
				{
					Weapon_System::GetInstance().DropCurrentWeapon();
				}
				Has_Dropped_Weapon = true;
			}
		}
	}
	else
	{
		if (Prev_Interact_Input)
		{
			if (Interact_Timer < 1.0f && !Has_Dropped_Weapon)
			{
				XMFLOAT3 pEyePos = Player_Camera_Get_Current_POS();
				XMFLOAT3 pDir = Player_Camera_Get_Front();

				ResourceItem* item = Resource_Manager::GetInstance().Get_Nearest_Weapon_In_View(pEyePos, pDir, 5.0f);

				if (item != nullptr)
				{
					if (Weapon_System::GetInstance().AddWeapon(item->W_Type))
					{
						item->Active = false;
						if (item->Drop_Box_Icon_Link)
						{
							item->Drop_Box_Icon_Link->Deactivate();
						}
					}
					else
					{
						Audio_Manager::GetInstance()->Play_SFX("Buffer_Denied");
					}
				}
			}
		}

		Interact_Timer = 0.0f;
		Has_Dropped_Weapon = false;
	}

	Prev_Interact_Input = Current_Input;
}

void Player_Update_Get_Reload_Input()
{
	if (KeyLogger_IsTrigger(KK_R) || XKeyLogger_IsPadTrigger(XINPUT_GAMEPAD_B))
	{
		Weapon_System::GetInstance().Reload();
	}
}

void Player_Update_Get_Mouse_Input()
{
	// Get Gamepad Input
	bool Is_Pad_Aim = XKeyLogger_IsPadPressed(XINPUT_GAMEPAD_RIGHT_THUMB) || XKeyLogger_IsPadPressed(XINPUT_GAMEPAD_LEFT_SHOULDER);
	static BYTE Prev_RT = 0;
	BYTE Curr_RT = XKeyLogger_GetRightTrigger();
	bool Is_RT_Pressed = (Curr_RT > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	bool Is_RT_Triggered = (Is_RT_Pressed) && (Prev_RT <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	Prev_RT = Curr_RT;


	// Get Now Weapom Info
	bool isAuto = false;

	if (Weapon_System::GetInstance().HasWeapon())
	{
		isAuto = Weapon_System::GetInstance().GetCurrentWeapon().Spec.IsAutomatic;
	}

	// Check Fire Input
	if (isAuto)
	{
		if (KeyLogger_IsMousePressed(Mouse_Button::LEFT) || Is_RT_Pressed)
		{
			Is_Shoot = true;
		}
		else
		{
			Is_Shoot = false;
		}
	}
	else
	{
		if (KeyLogger_IsMouseTrigger(Mouse_Button::LEFT) || Is_RT_Triggered)
		{
			Is_Shoot = true;
		}
		else
		{
			Is_Shoot = false;
		}
	}

	// Check Aim Input
	if (KeyLogger_IsMousePressed(Mouse_Button::RIGHT) || Is_Pad_Aim)
	{
		Is_Aiming_Mode = true;
	}
	else
	{
		Is_Aiming_Mode = false;
	}
}

// ----------------------------------------------------------------------------------------------------------------
//											--- Animation Model ---
// ----------------------------------------------------------------------------------------------------------------
void Player_Update_Change_Animation(float dt)
{
	// =====================================================================
	// Animation Swap Logic
	// Jump > Walk > Idle
	// =====================================================================

	if (Is_Player_Jump())
	{
		Model_Play_Animation(Player_Model, "Jump", false);
		//Audio_Manager::GetInstance()->Play_SFX("Player_Jump"); << Jump Sound
	}
	else if (Is_Player_Move())
	{
		Model_Play_Animation(Player_Model, "F_Move", true);
	}
	else
	{
		Model_Play_Animation(Player_Model, "Idle", true);
	}
}

// ----------------------------------------------------------------------------------------------------------------
//											--- Parameter Geter  ---
// ----------------------------------------------------------------------------------------------------------------
bool Is_Player_Move()
{
	return Is_Input_Moving;
}

bool Is_Player_Jump()
{
	return Is_Jump;
}

bool Is_Player_Shoot()
{
	return Is_Shoot;
}

bool Is_Plyer_Run()
{
	return Is_Run;
}