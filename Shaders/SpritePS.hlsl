Texture2D spriteTexture : register(t0);
SamplerState spriteSampler : register(s0);

cbuffer SpriteBuffer : register(b0)
{
    matrix worldViewProj;
    float4 tint;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = spriteTexture.Sample(spriteSampler, input.uv);
    return color * tint;
}
