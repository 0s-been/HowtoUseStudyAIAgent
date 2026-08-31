cbuffer SpriteBuffer : register(b0)
{
    matrix worldViewProj;
    float4 tint;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(float4(input.position, 1.0f), worldViewProj);
    output.uv = input.uv;
    return output;
}
