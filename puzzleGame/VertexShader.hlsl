cbuffer ConstantBuffer : register(b0)
{
    matrix WorldViewProjection;
};

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = mul(input.Pos, WorldViewProjection);
    output.Color = input.Color;
    return output;
}
