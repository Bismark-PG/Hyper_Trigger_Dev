/*==============================================================================

	Draw Cube [Cube.h]
	
	Author : Choi HyungJoon

==============================================================================*/
#ifndef CUBE_H
#define CUBE_H

#include <d3d11.h>
#include <DirectXMath.h>
#include "Shader_Manager.h"
#include "Collision.h"

enum class Cube_Type
{
	WALL,
	SILDE,
	BOX
};

void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Cube_Finalize(void);

void Cube_Draw(DirectX::XMMATRIX W_Matrix, Shader_Filter Filter = Shader_Filter::ANISOTROPIC, Cube_Type Type = Cube_Type::WALL);
void Cube_Draw_Shadow(DirectX::XMMATRIX W_Matrix, DirectX::XMMATRIX LightViewProj);
void Debug_Cube_Draw(DirectX::XMMATRIX W_Matrix);

ID3D11ShaderResourceView* Get_Cube_Type(Cube_Type Type);
AABB Get_Cube_AABB(DirectX::XMFLOAT3& POS);

#endif //CUBE_H