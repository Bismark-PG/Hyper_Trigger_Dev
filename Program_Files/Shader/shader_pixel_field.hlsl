/*==============================================================================

    フィールド描画用ピクセルシェーダー [shader_pixel_field.hlsl]

    Author : Choi HyungJoon

==============================================================================*/
// b0: Diffuse Light
cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float4 diffuse_color;
};

// b1: Ambient Light
cbuffer PS_CONSTANT_BUFFER : register(b1)
{
    float4 ambient_color;
};

// b2: Directional Light
cbuffer PS_CONSTANT_BUFFER : register(b2)
{
    float4 directional_world_vector;
    float4 directional_color;
};

// b3: Specular Light
cbuffer PS_CONSTANT_BUFFER : register(b3)
{
    float3 eye_posW;
    float specular_power = 30.0f;
    float4 specular_color = { 0.1, 0.1, 0.1, 1.0 };
};

struct PointLight
{
    float3 posW;
    float range;
    float4 color;
   
};

// b4: Point Light
cbuffer PS_CONSTANT_BUFFER : register(b4)
{
    PointLight point_light[4];
    int pointlight_count;
    float3 point_light_dummy;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;     // World POS
    float4 normalW : NORMAL0;    // World Normal
    float4 color : COLOR0;       // Color
    float2 uv : TEXCOORD0;       // Texcoord
    float4 posLight : POSITION1; // Light Wolrd POS
};

Texture2D tex;
Texture2D tex0 : register(t0);       //テクスチャ (Grass)
Texture2D tex1 : register(t1);       //テクスチャ (Rock)
Texture2D shadowMap : register(t2);  // Shadow Map Texture

SamplerState samp : register(s0);          //テクスチャサンプラ
SamplerState shadowSampler : register(s1); // Shadow Sampler

//=============================================================================
// Shadow Matrix Getter
//=============================================================================
float CalcShadowFactor(float4 posLight)
{
    // 1. Homogeneous Divide
    float3 projCoords = posLight.xyz / posLight.w;

    // 2. NDC (-1 ~ 1) -> Texture POS (0 ~ 1)
    // x : -1 ~ 1 -> 0 ~ 1
    // y : 1 ~ -1 -> 0 ~ 1 (Careful Y Axis Invert : DX Is Top-Down)
    projCoords.x =  projCoords.x * 0.5f + 0.5f;
    projCoords.y = -projCoords.y * 0.5f + 0.5f;

    // 3. Range Check
    if (projCoords.z > 1.0f || 
        projCoords.x < 0.0f || projCoords.x > 1.0f || 
        projCoords.y < 0.0f || projCoords.y > 1.0f)
    {
        return 1.0f;
    }

    // 4. Sampling Shadow Map (Use R Channel Have Depth)
    float closestDepth = shadowMap.Sample(shadowSampler, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    // 5. Bias
    float bias = 0.001f;

    // 6. If Current Depth Is Deeper, Will Be Shadow Area.
    float shadow = (currentDepth - bias) > closestDepth ? 0.0f : 1.0f;

    return shadow;
}

//=============================================================================
// Pixel Shader For Mash Field (Main)
//=============================================================================
float4 main(PS_IN pi) : SV_TARGET
{ 
    float2 uv;
    float angle = 3.141592f * 45 / 180.0f;
    uv.x = pi.uv.x * cos(angle) + pi.uv.y * sin(angle);
    uv.y = -pi.uv.x * sin(angle) + pi.uv.y * cos(angle);
    
    // Blender G and R
    float4 tex_color = tex0.Sample(samp, pi.uv) * pi.color.g + tex1.Sample(samp, pi.uv) * pi.color.r;
    
    // 材質の色
    float3 material_color = tex_color.rgb * diffuse_color.rgb;
    
    // Shadow (0.0 = Shadow, 1.0 = Light)
    float shadowFactor = CalcShadowFactor(pi.posLight);
    
    // Diffuse (並行光源)
    float4 normalW = normalize(pi.normalW);
    float dl = max(0, dot(-directional_world_vector, normalW));
    float3 diffuse = material_color * directional_color.rgb * dl * shadowFactor; // Set ShadowFactor
    
    // Ambient (環境光, アンビエントライト) Is Do Not Get Shadow
    float3 ambient = material_color * ambient_color.rgb;
    
    // Specular (スペキュラ)
    float3 toEye = normalize(eye_posW - pi.posW.xyz);
    float3 r = reflect(directional_world_vector, normalW).xyz;
    float t = pow(max(dot(r, toEye), 0.0f), specular_power);
    
    // Set ShadowFactor
    float3 specular = specular_color.rgb * t * shadowFactor;
    
    //最終カラー
    float3 color = ambient + diffuse + specular;
    
    for (int i = 0; i < pointlight_count; i++)
    {
        float3 lightToPixel = point_light[i].posW - pi.posW.xyz;
        float D = length(pi.posW.xyz - point_light[i].posW);
        float A = pow(max(1.0f - 1.0f / point_light[i].range * D, 0.0f), 2.0f);
        
        float dl = max(0, dot(normalize(lightToPixel), normalW.xyz));
        float3 diffuse = material_color * directional_color.rgb * dl;
      
        float3 r = reflect(normalize(lightToPixel), normalW.xyz).xyz;
        float t = pow(max(dot(r, toEye), 0.0f), specular_power);
        float3 specular = point_light[i].color.rgb * t;
        
        color += point_light[i].color.rgb * t;
    }
    
    return float4(color, 1.0f);
}