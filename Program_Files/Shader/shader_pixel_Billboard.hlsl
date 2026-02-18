/*==============================================================================

    ビルボード描画用ピクセルシェーダー [shader_pixel_Billboard.hlsl]

    Author : Choi HyungJoon

==============================================================================*/
struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pi) : SV_TARGET
{
    // Get Texture Color
    float4 finalColor = tex.Sample(samp, pi.texcoord);

    // Alpha Test (Discard Pixel If Alpha Is Less Than 0.1f)
    clip(finalColor.a - 0.1f);

    // Vertex Color Modulate
    finalColor *= pi.color;

    return finalColor;
}
