cbuffer ConstantBuffer : register(b0)
{
    matrix rotationMat;
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
    output.pos = mul(input.pos, rotationMat);
    output.color = input.color;
    return output;
}