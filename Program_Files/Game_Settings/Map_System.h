/*==============================================================================

	Manage Game World Map [Map_System.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef MAP_SYSTEM_H
#define MAP_SYSTEM_H
#include <DirectXMath.h>
#include "collision.h"
#include "model.h"

void Map_System_Initialize();
void Map_System_Finalize();

void Map_System_Update(double elapsed_time);

void Map_System_Draw();
void Map_System_Draw_Shadow(const DirectX::XMMATRIX& LightViewProj);

int Map_System_Get_Object_Count();

enum ObjectKind
{
	FIELD,
	WALL,
	BOX,
	SILDE,
	BALL
};

struct MapObject
{
	int OBJ_ID;
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 Rotation = { 0.0f, 0.0f, 0.0f };
	MODEL* ModelPtr = nullptr;
	AABB Collision;
};

const MapObject* Map_System_Get_Object(int index);
bool Map_System_Check_Collision_AABB(const AABB& Box);
#endif // MAP_SYSTEM_H
