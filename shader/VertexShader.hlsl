cbuffer ConstantBuffer : register(b0)
{
    float4x4 rotationMat;
};

struct VS_INPUT
{
    float4 pos : POSITION;
    float3 f_rest[16] : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

static const float SH_C0 = 0.2820947918f;
static const float SH_C1 = 0.4886025119f;
static const float SH_C2[5] = { 1.0925484306f, 1.0925484306f, 0.3153915653f, 1.0925484306f, 0.5462742153f };
static const float SH_C3[7] = { 0.5900435898f, 2.8906114426f, 0.4570457995f, 0.3731763325f, 0.4570457995f, 1.4453057213f, 0.5900435898f };

float3 computeColorFromSH(float3 position, float3 f_rest[16])
{
    float3 cam_pos = mul(rotationMat, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    float3 dir = position - cam_pos;
    dir = normalize(dir);

    float x = dir.x;
    float y = dir.y;
    float z = dir.z;

    float3 result = SH_C0 * f_rest[0];

    // Level 1
    result -= SH_C1 * y * f_rest[1];
    result += SH_C1 * z * f_rest[2];
    result -= SH_C1 * x * f_rest[3];

    // Level 2
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, yz = y * z, xz = x * z;

    result += SH_C2[0] * xy * f_rest[4];
    result += SH_C2[1] * yz * f_rest[5];
    result += SH_C2[2] * (2.0f * zz - xx - yy) * f_rest[6];
    result += SH_C2[3] * xz * f_rest[7];
    result += SH_C2[4] * (xx - yy) * f_rest[8];

    // Level 3
    result += SH_C3[0] * y * (3.0f * xx - yy) * f_rest[9];
    result += SH_C3[1] * xy * z * f_rest[10];
    result += SH_C3[2] * y * (4.0f * zz - xx - yy) * f_rest[11];
    result += SH_C3[3] * z * (2.0f * zz - 3.0f * xx - 3.0f * yy) * f_rest[12];
    result += SH_C3[4] * x * (4.0f * zz - xx - yy) * f_rest[13];
    result += SH_C3[5] * z * (xx - yy) * f_rest[14];
    result += SH_C3[6] * x * (xx - 3.0f * yy) * f_rest[15];

    result += 0.5f;
    return result;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(rotationMat, input.pos);
    output.color = float4(computeColorFromSH(input.pos.xyz, input.f_rest), 1.0f);
    return output;
}
