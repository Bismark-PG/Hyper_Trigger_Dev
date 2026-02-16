/*==============================================================================

    3D描画用頂点シェーダー [shader_vertex_3d.hlsl]

    Author : Choi HyungJoon

==============================================================================*/

// Buffer For C++ input (Matrix)
// b0: World Matrix
cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 world;
};

// b1: View Matrix
cbuffer VS_CONSTANT_BUFFER : register(b1)
{
    float4x4 view;
};

// b2: Projection Matrix
cbuffer VS_CONSTANT_BUFFER : register(b2)
{
    float4x4 proj;
};

// b3 : Shadow Map
cbuffer VS_CONSTANT_SHADOW : register(b3)
{
    float4x4 dummy_world;
    float4x4 LightViewProjection;
};

// Struct for VS shader
struct VS_IN
{
    float4 posL : POSITION0;    // Local POS
    float4 normalL : NORMAL0;   // Local Normal
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;   // Transformed POS for rasterizer
    float4 posW : POSITION0;     // World POS (For PS)
    float4 normalW : NORMAL0;    // World Normal (For PS)
    float4 color : COLOR0;       // Vertex Color
    float2 texcoord : TEXCOORD0; // texcoord POS
    float4 posLight : POSITION1; // Light POS
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
    float4 posW = mul(vi.posL, world);
    
    vo.posW = posW;
    
    // プロジェクション変換
    vo.posH = mul(vi.posL, mtxWVP);
   
    // Light View-Proj Conversion For Shadow POS
    vo.posLight = mul(posW, LightViewProjection);
    
    //--- 2.  Input Data For PS ---
    // Input World POS and Normal for Light Calculation 
    //ワールド変換行列の転置逆行列を使う
    float4 normalW = mul(float4(vi.normalL.xyz, 0.0f), world);
    //αは0
    vo.normalW = normalize(normalW); //単位ベクトル化    
    vo.posW = mul(vi.posL, world);
    
    // (Color and UV is same)
    vo.color = vi.color;
    vo.texcoord = vi.texcoord;
    
    return vo;
}