/*==============================================================================

	Draw Cube [Cube.cpp]
	
	Author : Choi HyungJoon

==============================================================================*/

#include "Cube.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "Texture_Manager.h"
#include "Shader_Manager.h"

static constexpr int NUM_VERTEX = 4 * 6; // 頂点数
static constexpr int NUM_INDEX  = 3 * 2 * 6; // Count Of Index

static ID3D11Buffer* Vertex_Buffer = nullptr; // 頂点バッファ
static ID3D11Buffer* Index_Buffer = nullptr; // 頂点バッファ

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static int Cube_Tex_ID = -1;

//3D頂点構造体
struct Vertex3D
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT3 normal;   // 法線
	XMFLOAT4 color;    // 色
	XMFLOAT2 texcord;  // UV
};

static Vertex3D Cube_Vertex[24]
{
	//前
	{{-0.5f, 0.5f,-0.5f}, {0.0f,0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f,  0.0f }}, // 0
	{{ 0.5f,-0.5f,-0.5f}, {0.0f,0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.25f}}, // 1
	{{-0.5f,-0.5f,-0.5f}, {0.0f,0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f,  0.25f}}, // 2
//	{{-0.5f, 0.5f,-0.5f}, {0.0f,0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f,  0.0f }},
	{{ 0.5f, 0.5f,-0.5f}, {0.0f,0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.0f }}, // 3
//	{{ 0.5f,-0.5f,-0.5f}, {0.0f,0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.25f}},
	//後ろ
	{{ 0.5f, 0.5f, 0.5f}, {0.0f,0.0f,1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.0f }},
	{{-0.5f,-0.5f, 0.5f}, {0.0f,0.0f,1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.75f, 0.25f}},
	{{ 0.5f,-0.5f, 0.5f}, {0.0f,0.0f,1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.25f}},
//	{{ 0.5f, 0.5f, 0.5f}, {0.0f,0.0f,1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.0f }},
	{{-0.5f, 0.5f, 0.5f}, {0.0f,0.0f,1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.75f, 0.0f }},
//	{{-0.5f,-0.5f, 0.5f}, {0.0f,0.0f,1.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.75f, 0.25f}},
	//右
	{{ 0.5f, 0.5f,-0.5f}, {1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.0f }},
	{{ 0.5f,-0.5f, 0.5f}, {1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.25f}},
	{{ 0.5f,-0.5f,-0.5f}, {1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.25f}},
//	{{ 0.5f, 0.5f,-0.5f}, {1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.0f }},
	{{ 0.5f, 0.5f, 0.5f}, {1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.0f }},
//	{{ 0.5f,-0.5f, 0.5f}, {1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.25f}},
	//左
	{{-0.5f, 0.5f, 0.5f}, {-1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.75f, 0.0f }},
	{{-0.5f,-0.5f,-0.5f}, {-1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f,  0.25f}},
	{{-0.5f,-0.5f, 0.5f}, {-1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.75f, 0.25f}},
//	{{-0.5f, 0.5f, 0.5f}, {-1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.75f, 0.0f }},
	{{-0.5f, 0.5f,-0.5f}, {-1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f,  0.0f }},
//	{{-0.5f,-0.5f,-0.5f}, {-1.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {1.0f,  0.25f}},
	//上
	{{-0.5f, 0.5f, 0.5f}, {0.0f,1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f,  0.25f }},
	{{ 0.5f, 0.5f,-0.5f}, {0.0f,1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.50f}},
	{{-0.5f, 0.5f,-0.5f}, {0.0f,1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f,  0.50f}},
//	{{-0.5f, 0.5f, 0.5f}, {0.0f,1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.0f,  0.25f }},
	{{ 0.5f, 0.5f, 0.5f}, {0.0f,1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.25f }},
//	{{ 0.5f, 0.5f,-0.5f}, {0.0f,1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.50f}},
	//下
	{{ 0.5f,-0.5f, 0.5f}, {0.0f,-1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.25f }},
	{{-0.5f,-0.5f,-0.5f}, {0.0f,-1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.50f}},
	{{ 0.5f,-0.5f,-0.5f}, {0.0f,-1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.50f}},
//	{{ 0.5f,-0.5f, 0.5f}, {0.0f,-1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.25f, 0.25f }},
	{{-0.5f,-0.5f, 0.5f}, {0.0f,-1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.25f }},
//	{{-0.5f,-0.5f,-0.5f}, {0.0f,-1.0f,0.0f}, {1.0f,1.0f,1.0f,1.0f}, {0.50f, 0.50f}},
};

// 65,536 Is Full Size
static unsigned short Cube_Index[]
{
	 0,  1,  2,  0,  3,  1,
	 4,  5,  6,  4,  7,  5,
	 8,  9, 10,  8, 11,  9,
	12, 13, 14, 12, 15, 13,
	16, 17, 18, 16, 19, 17,
	20, 21, 22, 20, 23, 21
};

void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3D) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = Cube_Vertex;

	g_pDevice->CreateBuffer(&bd, &sd, &Vertex_Buffer);

	// Creat Index Buffer
	bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX; // if Arr >> "sizeof(Cube_Index)" is OK
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	sd.pSysMem = Cube_Index;

	g_pDevice->CreateBuffer(&bd, &sd, &Index_Buffer);

	// Cube_Tex_ID = Texture_Load(L"sample.png");
}

void Cube_Finalize(void)
{
	SAFE_RELEASE(Vertex_Buffer);
	SAFE_RELEASE(Index_Buffer);
}

void Cube_Draw(XMMATRIX W_Matrix, Shader_Filter Filter, Cube_Type Type)
{
	// シェーダーを描画パイプラインに設定
	Shader_Manager::GetInstance()->Begin3D(Filter);

	// Color Reset
	Shader_Manager::GetInstance()->SetDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// Texture_SetTexture(Cube_Tex_ID);
	ID3D11ShaderResourceView* SRV = Get_Cube_Type(Type);
	Shader_Manager::GetInstance()->SetTexture3D(SRV);


	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &Vertex_Buffer, &stride, &offset);

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	// Set Index Buffer
	// unsigned short = DXGI_FORMAT_R16_UINT
	// unsigned int	  = DXGI_FORMAT_R32_UINT
	g_pContext->IASetIndexBuffer(Index_Buffer, DXGI_FORMAT_R16_UINT, 0);

	// 頂点シェーダーに座標変換行列を設定
	Shader_Manager::GetInstance()->SetWorldMatrix3D(W_Matrix);

	// Just Draw
	// g_pContext->Draw(NUM_VERTEX, 0); 
	
	// Draw With Index
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}

void Cube_Draw_Shadow(DirectX::XMMATRIX W_Matrix, DirectX::XMMATRIX LightViewProj)
{
	// Send Matrix To Shader
	Shader_Manager::GetInstance()->SetShadowWorldMatrix(W_Matrix, LightViewProj);

	// Set Vertex, Index Buffer
	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &Vertex_Buffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(Index_Buffer, DXGI_FORMAT_R16_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}

void Debug_Cube_Draw(DirectX::XMMATRIX W_Matrix)
{
	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &Vertex_Buffer, &stride, &offset);

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set Index Buffer
	// unsigned short = DXGI_FORMAT_R16_UINT
	// unsigned int	  = DXGI_FORMAT_R32_UINT
	g_pContext->IASetIndexBuffer(Index_Buffer, DXGI_FORMAT_R16_UINT, 0);

	// 頂点シェーダーに座標変換行列を設定
	Shader_Manager::GetInstance()->SetWorldMatrix3D(W_Matrix);

	// Draw With Index
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}

ID3D11ShaderResourceView* Get_Cube_Type(Cube_Type Type)
{
	ID3D11ShaderResourceView* SRV = {};

	switch (Type)
	{
	case Cube_Type::WALL:
		SRV = Texture_Manager::GetInstance()->
			Get_Shader_Resource_View(Texture_Manager::GetInstance()->GetID("Mash"));
		break;

	case Cube_Type::SILDE:
		SRV = Texture_Manager::GetInstance()->
			Get_Shader_Resource_View(Texture_Manager::GetInstance()->GetID("Cube_Sample"));
		break;	

	case Cube_Type::BOX:
		SRV = Texture_Manager::GetInstance()->
			Get_Shader_Resource_View(Texture_Manager::GetInstance()->GetID("Box_Sample"));
		break;
	}
	return SRV;
}

AABB Get_Cube_AABB(DirectX::XMFLOAT3& POS)
{
	float Collision = 0.5f;

	return AABB
	{
		{ POS.x + Collision, POS.y + Collision, POS.z + Collision }, // Max
		{ POS.x - Collision, POS.y - Collision, POS.z - Collision }  // Min
	};
}

/*
>> Note : XMMatrix Guide, Pyramid

ワールド座標変換行列の作成
XMMATRIX mtxWorld = XMMatrixIdentity(); //単位行列の作成
XMMATRIX mtxWorld = XMMatrixTranslation(-4.5f, 0.5f, 4.5f); // 単位行列

XMMATRIX mtxWorld = XMMatrixRotationX(XMConvertToRadians(45.0f)); // 回転 X
XMMATRIX mtxWorld = XMMatrixRotationY(XMConvertToRadians(45.0f)); // 回転 Y
XMMATRIX mtxWorld = XMMatrixRotationZ(XMConvertToRadians(45.0f)); // 回転 Z

XMMATRIX mtxWorld = XMMatrixScaling(1.0f, 3.0f, 1.0f); // Sacleing (X, Y, Z)

float x = 0.5;
for (float i = 10.0f; i > 0; i -= 1.0f)
{
	XMMATRIX mtxOffsec = XMMatrixTranslation(0, x, 0);
	XMMATRIX mtxRotateY = XMMatrixRotationY(XMConvertToRadians(g_Angle));
	XMMATRIX mtxScale = XMMatrixScaling(i, 0.5f, i);

	XMMATRIX mtxWorld = mtxOffsec * mtxScale * mtxRotateY;

	 頂点シェーダーに座標変換行列を設定
	Shader_3D_SetWorldMatrix(mtxWorld);

	 ポリゴン描画命令発行
	g_pContext->Draw(NUM_VERTEX, 0);

	x += 1.0f;
}

>> Pyramid <<
int d = 1, i = 5;
for (int j = 0; j < 2; j++)
{
	for (int k = 0; k < 2; k++)
	{
		XMMATRIX mtxOffsec = XMMatrixTranslation(-2.5f + i * j, 0.0f, 2.5f - i * k);
		
		for (int y = 0; y < 5 * d; y++)
		{
			for (int z = 0; z < 5 * d - y; z++)
			{
				for (int x = 0; x < 5 * d - y; x++)
				{
					XMMATRIX mtxTrans = XMMatrixTranslation(-2.0f * d + x + 0.5f * y, 0.5f + y, -2.0f * d + z + 0.5f * y);
					XMMATRIX mtxRotateY = XMMatrixRotationY(XMConvertToRadians(g_Angle));
					XMMATRIX mtxScale = XMMatrixScaling(1.0f, g_Scale, 1.0f);
					
					XMMATRIX mtxWorld = mtxTrans * mtxRotateY * mtxOffsec * mtxScale;
					
					Cube_Draw(mtxWorld);
				}
			}
		}
	}
}
*/