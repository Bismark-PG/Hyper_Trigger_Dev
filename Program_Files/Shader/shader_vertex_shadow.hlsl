/*==============================================================================

    Vertex Shader For Static Object [shader_vertex_shadow.hlsl]

    Author : Choi HyungJoon

==============================================================================*/

cbuffer VS_SHADOW_BUFFER : register(b0)
{
    float4x4 World;
    float4x4 LightViewProjection; // Light View * Light Proj
};

struct VS_IN
{
    float4 posL : POSITION0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;
    
    // --- Get Matrix ---
    // World -> Light View -> Light Projection
    float4 posW = mul(vi.posL, World);
    vo.posH = mul(posW, LightViewProjection);

    return vo;
}