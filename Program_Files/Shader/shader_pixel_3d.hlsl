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

cbuffer PS_SHADOW_PROP : register(b5)
{
    float PCF_Spread;       // PCF Spread (0.0f ~ 1.0f) : 0.0f = Sharp Shadow, 1.0f = Soft Shadow
    int PCF_Loop;           // PCF Loop Count (1 ~ 5) : 1 = Sharp Shadow, 5 = Soft Shadow
    float2 Padding_Shadow;  // Set 16Byte Alignment
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

Texture2D shadowMap : register(t4);         // Shadow Map Texture
SamplerState shadowSampler : register(s1);  // Shadow Sampler

//=============================================================================
// Shadow Matrix Getter
//=============================================================================
float CalcShadowFactor(float4 posLight)
{
    // 1. Perspective Divide
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

    // 4. Percentage Closer Filtering
    float shadowSum = 0.0f;
    float currentDepth = projCoords.z;
    
    // 5. Bias Shadow For Delete Shadow Acne
    float bias = 0.002f; // 0.002f ~ 0.005f
    
    // Texel Size: Inverse Of Shadow Map Resolution (Based On Shadow_Manager.h)
    // 1.0f / 4096.0f : Sharp Shadow
    // 2.0f / 4096.0f : Soft Shadow 
    
    // float2 texelSize = PCF_Spread / 4096.0f;
    float2 texelSize = 1.0f / 4096.0f;
    // int loopRange = PCF_Loop;
    int loopRange = 2;
    
    float sampleCount = 0.0f;
    
    // [unroll] : Unroll Loop For Performance Optimization
    // [Loop]   : Allow Dynamic Loop Count
    // [loop] for (int x = -PCF_Loop; x <= PCF_Loop; ++x)
    [unroll]
    for (int x = -loopRange; x <= loopRange; ++x)
    {
        // [loop] for (int y = -PCF_Loop; y <= PCF_Loop; ++y)
        [unroll]
        for (int y = -loopRange; y <= loopRange; ++y)
        {
            float pcfDepth = shadowMap.SampleLevel(shadowSampler, projCoords.xy + float2(x, y) * texelSize, 0).r;
            
            // Depth Comparison :
            // If Current Depth Is Greater Than Recorded Depth, Not Shadow (1.0)
            // If Current Depth Is Greater Than shadowMap, Shadow (0.0)
            shadowSum += (currentDepth - bias < pcfDepth) ? 1.0f : 0.0f;
            //sampleCount += 1.0f;
            sampleCount += 1.0f;
        }
    }
    
    // Return Average Of Samples For Soft Shadow (0.0 ~ 1.0)
    return shadowSum / sampleCount;
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