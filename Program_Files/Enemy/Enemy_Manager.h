/*==============================================================================

	Manage Enemy Spawn System [Enemy_Manager.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include "Enemy.h"
#include <vector>

class Enemy_Manager
{
public:
	static Enemy_Manager& GetInstance()
	{
		static Enemy_Manager instance;
		return instance;
	}

	void Init();
	void Final();

	void Update(double elapsed_time);

	void Draw();
	void Draw_Shadow(const DirectX::XMMATRIX& LightViewProj);

	void Reset();

	// Call to spawn an enemy
	void Spawn(EnemyType type);

	std::vector<Enemy*>& Get_Active_List()
	{ 
		return m_ActiveList;
	}

	// AABB Collision Check for Enemy
	Enemy* Check_Collision_AABB(const AABB& box);

private:
	Enemy_Manager() = default;
	~Enemy_Manager() = default;

	Enemy_Manager(const Enemy_Manager&) = delete;
	Enemy_Manager& operator=(const Enemy_Manager&) = delete;

	std::vector<Enemy*> m_EnemyPool; 
	std::vector<Enemy*> m_ActiveList;
};
#endif // ENEMY_MANAGER_H