/*==============================================================================

    Vertex Shader For Animation [shader_vertex_shadow_animation.hlsl]

    Author : Choi HyungJoon

==============================================================================*/

cbuffer VS_SHADOW_BUFFER : register(b0)
{
    float4x4 World;
    float4x4 LightViewProjection; // Light View * Light Proj
};

// Buffer For Animation Skinning
cbuffer VS_SKIN_BUFFER : register(b3)
{
    float4x4 boneTransforms[256];
};

struct VS_IN
{
    float4 posL : POSITION0;
    
    // Use For Animation Skinning
    uint4 boneIndices : BLENDINDICES0;
    float4 boneWeights : BLENDWEIGHT0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION; // Shadow Map POS
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;

    // Set Animation Input
    float4 posAccum = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // If Weight Is Over Than 0, It Is Animation
    if (vi.boneWeights.x + vi.boneWeights.y + vi.boneWeights.z + vi.boneWeights.w > 0.0f)
    {
        float4x4 boneMtx;
        
        boneMtx = boneTransforms[vi.boneIndices.x];
        posAccum += mul(vi.posL, boneMtx) * vi.boneWeights.x;
        
        boneMtx = boneTransforms[vi.boneIndices.y];
        posAccum += mul(vi.posL, boneMtx) * vi.boneWeights.y;
        
        boneMtx = boneTransforms[vi.boneIndices.z];
        posAccum += mul(vi.posL, boneMtx) * vi.boneWeights.z;
        
        boneMtx = boneTransforms[vi.boneIndices.w];
        posAccum += mul(vi.posL, boneMtx) * vi.boneWeights.w;
        
        posAccum.w = 1.0f;
    }
    else
    {
        // No Skinning Data, Just Use Local Data
        posAccum = vi.posL;
    }
    
    if (posAccum.w == 0.0f)
    {
        posAccum = vi.posL;
        posAccum.w = 1.0f; 
    }

    // --- Get Matrix ---
    // World -> Light View -> Light Projection
    float4 posW = mul(posAccum, World);
    vo.posH = mul(posW, LightViewProjection);

    return vo;
}