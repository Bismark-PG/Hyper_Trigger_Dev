/*==============================================================================

	Manage Game World Map [Map_System.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "Map_System.h"
#include "Game_Model.h"
#include "Cube.h"
#include "Mash_Field.h"
#include "Debug_Collision.h"
#include "debug_ostream.h"
#include "Game.h"
#include "Billboard_Manager.h"
#include "Light_Manager.h"
#include "Player.h"
using namespace DirectX;

static MapObject Map_OBJ[]
{
	// (Ground)
	{FIELD, { 0.0f, 0.0f, 0.0f }, {}, {}, {{-250.0f, -1.0f, -250.0f}, {250.0f, 0.0f, 250.0f}}},

	{WALL, { -10.0f, 1.5f, -10.0f }}, {WALL, { -10.0f, 1.5f,  10.0f }},
	{WALL, {  10.0f, 1.5f, -10.0f }}, {WALL, {  10.0f, 1.5f,  10.0f }},

	{WALL, { -20.0f, 0.5f, 0.0f }},
	{WALL, { -23.0f, 1.5f, 0.0f }},
	{WALL, { -26.0f, 2.5f, 0.0f }},
	{WALL, { -29.0f, 3.5f, 0.0f }},
	{WALL, { -32.0f, 3.5f, 0.0f }},

	{WALL, { 20.0f, 1.0f, 0.0f }},
	{WALL, { 25.0f, 3.0f, 0.0f }},

	{WALL, { 0.0f, 2.5f, 30.0f }}, {WALL, { 0.0f, 5.5f, 30.0f }},
	{WALL, { 0.0f, 2.5f, -30.0f }}, {WALL, { 0.0f, 5.5f, -30.0f }},

	{WALL, { 0.0f, 0.0f, 20.0f }, { XMConvertToRadians(30.0f), 0.0f, 0.0f }}
};

void Map_System_Initialize()
{
	for (MapObject& o : Map_OBJ)
	{
		// Get Model Pointer
		switch (o.OBJ_ID)
		{
		case BALL:
			o.ModelPtr = Model_Manager::GetInstance()->GetModel("Ball");
			break;
		default:
			o.ModelPtr = nullptr;
			break;
		}

		// Get AABB
		switch (o.OBJ_ID)
		{
		case WALL:
		case SILDE:
			o.Collision = Get_Cube_AABB(o.Position);
			break;
		case FIELD:
			break;
		default:
			if (o.ModelPtr != nullptr)
				o.Collision = Model_Get_AABB(o.ModelPtr, o.Position);
			break;
		}
	}

	Billboard_Manager::Instance().Create({ -5.0f, 0.0f, -5.0f }, Billboard_Type::OBJECT);
}

void Map_System_Finalize()
{
}

void Map_System_Update(double elapsed_time)
{
	// --- Update Global Light ---
	Light_Manager::GetInstance().Global_Light_Update(elapsed_time);

	// --- Set Player Light ---
	Light_Manager::GetInstance().Set_Point_Light_Active_Count(1);
	DirectX::XMFLOAT3 playerPos = Player_Get_POS();
	playerPos.y += 1.0f;
	Light_Manager::GetInstance().Set_Point_Light(0, playerPos, 3.0f, { 1.0f, 0.5f, 0.0f, 1.0f });
}

void Map_System_Draw()
{
	XMMATRIX mtxWorld = {};

	for (const MapObject& o : Map_OBJ)
	{
		mtxWorld = XMMatrixIdentity(); // Defense Code

		switch (o.OBJ_ID)
		{
		case FIELD:
			Mash_Field_Draw();
			break;

		case BALL:
			if (o.ModelPtr != nullptr)
			{
				mtxWorld = XMMatrixTranslation(o.Position.x, o.Position.y, o.Position.z);
				ModelDraw(o.ModelPtr, mtxWorld);
			}
			else
			{
				Debug::D_Out << "Failed to Load Model Pointer : " << o.OBJ_ID << std::endl;
				return;
			}
			break;

		default: // Cube
			//ここはライトのコード入れる
			mtxWorld = XMMatrixTranslation(o.Position.x, o.Position.y, o.Position.z);

			if (o.OBJ_ID == SILDE)
				Cube_Draw(mtxWorld, {}, Cube_Type::SILDE);
			else if (o.OBJ_ID == WALL)
				Cube_Draw(mtxWorld);
			break;
		}

		if (Get_Debug_Mode_State())
		{
			if (o.OBJ_ID != FIELD)
			{
				Debug_Collision_Draw(o.Collision, { 1.0f, 0.0f, 0.5f, 1.0f });
			}
		}
	}
}

void Map_System_Draw_Shadow(const DirectX::XMMATRIX& LightViewProj)
{
	XMMATRIX mtxWorld = {};

	for (const MapObject& o : Map_OBJ)
	{
		if (o.OBJ_ID == FIELD) continue; // Field Is Do Not Draw Shadow, Just Get

		mtxWorld = XMMatrixIdentity();

		switch (o.OBJ_ID)
		{
		case BALL:
			if (o.ModelPtr != nullptr)
			{
				mtxWorld = XMMatrixTranslation(o.Position.x, o.Position.y, o.Position.z);
				Shader_Manager::GetInstance()->DrawModelShadow(o.ModelPtr, mtxWorld, LightViewProj);
			}
			break;

		case WALL:
		case SILDE:
		case BOX:
			mtxWorld = XMMatrixTranslation(o.Position.x, o.Position.y, o.Position.z);
			// Draw Cube Shadow (Use Cube_Draw_Shadow In Cube.cpp)
			Cube_Draw_Shadow(mtxWorld, LightViewProj);
			break;
		}
	}
}

int Map_System_Get_Object_Count()
{
	return sizeof(Map_OBJ) / sizeof(Map_OBJ[0]);
}

const MapObject* Map_System_Get_Object(int index)
{
	return &Map_OBJ[index];
}

bool Map_System_Check_Collision_AABB(const AABB& Box)
{
	for (const auto& o : Map_OBJ)
	{
		// Ignore Field
		if (o.OBJ_ID == FIELD) 
		{
			continue;
		}

		// AABB vs AABB Check
		if (Collision_Is_Hit_AABB(Box, o.Collision).Is_Hit)
		{
			return true; // Collision Detected
		}
	}
	return false; // No Collision
}