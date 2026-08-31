/*==============================================================================

	Manage Enemy Type [Enemy_Type.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef ENEMY_TYPE_H
#define ENEMY_TYPE_H
enum class EnemyType
{
	GROUND_NORMAL,
	GROUND_DASHER,
	GROUND_TANKER,
	FLIGHT_NORMAL,
	FLIGHT_DASHER
};

struct Enemy_Info
{
	EnemyType Type;
	int HP, MaxHP;
	float Speed, Dash_Speed;
	float Scale;
	float Fire_Range, Bullet_Speed, Fire_Interval;
	float EXP_Mount;

	int Collision_Damage;
	int Bullet_Damage;   
};

// --- Ground Type--- 
static const Enemy_Info Ground_Normal_Info
{ //Enemy Type, HP, M_HP, Speed, Dash Speed, Scale, Range, Speed, Interval, EXP Mount
	EnemyType::GROUND_NORMAL, 50, 50, 2.0f, 3.0f, 5.0f, 2.0f, 1.0f, 2.0f, 10.0f,
  //Collision_Damage, Bullet_Damage
	//10, 10
	1, 1
};

static const Enemy_Info Ground_Dasher_Info
{ //Enemy Type, HP, M_HP, Speed, Dash Speed, Scale, Range, Speed, Interval, EXP Mount
	EnemyType::GROUND_DASHER, 10, 10, 5.0f, 10.0f, 3.0f, 10.0f, 1.0f, 2.0f, 25.0f,
	//Collision_Damage, Bullet_Damage
	//20, 20
	2, 2
};

static const Enemy_Info Ground_Tanker_Info
{ //Enemy Type, HP, M_HP, Speed, Dash Speed, Scale, Range, Speed, Interval, EXP Mount
	EnemyType::GROUND_TANKER, 100, 100, 1.0f, 2.0f, 10.0f, 2.0f, 1.0f, 2.0f, 15.0f,
	//Collision_Damage, Bullet_Damage
	//25, 25
	5, 5
};

// --- Filght Type--- 
static const Enemy_Info Filght_Normal_Info
{ //Enemy Type, HP, M_HP, Speed, Dash Speed, Scale, Range, Speed, Interval, EXP Mount
	EnemyType::FLIGHT_NORMAL, 25, 25, 5.0f, 8.0f, 1.5f, 7.5f, 1.0f, 2.0f, 15.0f,
	//Collision_Damage, Bullet_Damage
	//10, 25
	1, 2
};

static const Enemy_Info Filght_Dasher_Info
{ //Enemy Type, HP, M_HP, Speed, Dash Speed, Scale, Range, Speed, Interval, EXP Mount
	EnemyType::FLIGHT_DASHER, 5, 5, 4.0f, 6.0f, 2.0f, 10.0f, 1.0f, 2.0f, 30.0f,
	//Collision_Damage, Bullet_Damage
	//50, 25
	5, 2
};

#endif // ENEMY_TYPE_H