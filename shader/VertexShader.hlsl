cbuffer ConstantBuffer : register(b0)
{
    float4x4 rotationMat;
};

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color: COLOR;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(rotationMat, input.pos);
    output.color = input.color;
    return output;
}