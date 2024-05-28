// Constant buffer for the vertex shader
cbuffer ConstantBuffer : register(b0)
{
    matrix Final;
};

// Output structure for the vertex shader
struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// Vertex shader
VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
    VOut output;

    // Apply the transformation matrix to the position
    output.position = mul(position, Final);
    output.color = color;

    return output;
}

// Pixel shader
float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}
