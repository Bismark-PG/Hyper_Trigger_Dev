/*==============================================================================

    ビルボード描画用頂点シェーダー [shader_vertex_Billboard.hlsl]

    Author : Choi HyungJoon

==============================================================================*/

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 world;
};

cbuffer VS_CONSTANT_BUFFER : register(b1)
{
    float4x4 view;
};

cbuffer VS_CONSTANT_BUFFER : register(b2)
{
    float4x4 proj;
};

cbuffer VS_CONSTANT_BUFFER : register(b3)
{
    float2 scale;
    float2 translation;
};

struct VS_IN
{
    float4 posL : POSITION0;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

//=============================================================================
// 頂点シェーダ (Main)
//=============================================================================
VS_OUT main(VS_IN vi)
{
    VS_OUT vo;
    
    //--- 1. Transformation POS ---
    //座標変換 (WVP)
    float4x4 mtxWV = mul(world, view); //ビュー変換
    float4x4 mtxWVP = mul(mtxWV, proj);
    // プロジェクション変換
    vo.posH = mul(vi.posL, mtxWVP);
    
    // (Color and UV is same)
    vo.color = vi.color;
    vo.texcoord = vi.texcoord * scale * translation;
    
    return vo;
}