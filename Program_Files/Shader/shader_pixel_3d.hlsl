/*==============================================================================

    3D描画用ピクセルシェーダー [shader_pixel_3d.hlsl]

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
    float3 eye_posW;        // Camera World POS
    float specular_power;   // Sharpness of Reflection Highlights
    float4 specular_color;  // Reflection Highlights Color (ex: { 0.1, 0.1, 0.1, 1.0 })
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
    int point_light_count;
    float3 point_light_dummy; //float4つ分ずつ送るためのdummy
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;     // World POS
    float4 normalW : NORMAL0;    // World Normal
    float4 color : COLOR0;       // Color
    float2 texcoord : TEXCOORD0; // Texcoord
    float4 posLight : POSITION1; // Light Wolrd POS
};

Texture2D tex;     //テクスチャ
SamplerState samp; //テクスチャサンプラ

Texture2D shadowMap : register(t2);         // Shadow Map Texture
SamplerState shadowSampler : register(s1);  // Shadow Sampler

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
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = -projCoords.y * 0.5f + 0.5f;
    
    // 3. Range Check (Out Of Screen, Do Not Get Light
    if (projCoords.z > 1.0f ||
        projCoords.x < 0.0f || projCoords.x > 1.0f || 
        projCoords.y < 0.0f || projCoords.y > 1.0f)
    {
        return 1.0f; // Get Light
    }
    
    // 4. Sampling Shadow Map (Use R Channel Have Depth)
    float closestDepth = shadowMap.Sample(shadowSampler, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    // 5. Bias Sahdow For Delete Shadow Acne
    float bias = 0.001f;
    
    // 6. If Current Depth Is Deeper, Will Be Shadow Area.
    float shadow = (currentDepth - bias) > closestDepth ? 0.0f : 1.0f;
    
    return shadow;
}

//=============================================================================
// ピクセルシェーダ (Main)
//=============================================================================
float4 main(PS_IN pi) : SV_TARGET
{
    //材質
    float3 material_color = tex.Sample(samp, pi.texcoord).rgb * pi.color.rgb * diffuse_color.rgb;

    // Shadow (0.0 = Shadow, 1.0 = Light)
    float shadowFactor = CalcShadowFactor(pi.posLight);
    
    //並行光源(ディフューズライト)
    float4 normalW = normalize(pi.normalW);
    //float dl = max(0.0f, dot(-directional_world_vector, normalW));
    float dl = (dot(-directional_world_vector, normalW) + 1.0f) * 0.5f;
    float3 diffuse = material_color * directional_color.rgb * dl * shadowFactor;
    
    //環境光(アンビエントカラー(ライト))
    float3 ambient = material_color * ambient_color.rgb;

    //スペキュラライト
    float3 toEye = normalize(eye_posW - pi.posW.xyz);
    float3 r = reflect(directional_world_vector, normalW).xyz;
    // float3 r = reflect(normalize(directional_world_vector), normalW).xyz;
    float t = pow(max(dot(r, toEye), 0.0f), specular_power);
    float3 specular = specular_color.rgb * t * shadowFactor;

    float alpha = tex.Sample(samp, pi.texcoord).a * diffuse_color.a * pi.color.a;
    float3 color = ambient + diffuse + specular;
    //最終的に我々の目に届く色
    
    //リムライト
    //float lim = 1.0f-max(dot(normalW.xyz, toEye), 0.0f);
    //lim = pow(lim, 5.0f);
    //color += float3(lim, lim, lim);
    
    for (int i = 0; i < point_light_count; i++)
    {
        //点光源(ポイントライト)
        //面(ピクセル)とライトとの距離を測る
        float D = length(pi.posW.xyz - point_light[i].posW);
        //影響力の計算
        float A = pow(max(1.0f - 1.0f / point_light[i].range * D, 0.0f), 2.0f);
        color += point_light[i].color.rgb * material_color * A;
    }
    
    return float4(color, alpha); //uvの座標のサンプラーのテクスチャの色を返す
}