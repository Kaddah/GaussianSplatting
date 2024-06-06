<<<<<<< Updated upstream
=======
cbuffer ConstantBuffer : register(b0)
{
    float4x4 rotationMat;
    float sh[45];
};

>>>>>>> Stashed changes
struct VS_INPUT
{
    float3 pos : POSITION;
    float4 color: COLOR;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

// Coefficients
static const float SH_C0 = 0.2820947918f;
static const float SH_C1 = 0.4886025119f;
static const float SH_C2[5] = {1.0925484306f, 1.0925484306f, 0.3153915653f, 1.0925484306f, 0.5462742153f};
static const float SH_C3[7] = {0.5900435898f, 2.8906114426f, 0.4570457995f, 0.3731763325f, 0.4570457995f, 1.4453057213f, 0.5900435898f};

float3 computeColorFromSH(float3 position)
{
    float3 cam_pos = mul(rotationMat, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    float3 dir = position - cam_pos;
    dir = normalize(dir);

    float x = dir.x;
    float y = dir.y;
    float z = dir.z;

    float3 result = SH_C0 * float3(sh[0], sh[1], sh[2]);

    // Level 1
    result -= SH_C1 * y * float3(sh[3], sh[4], sh[5]);
    result += SH_C1 * z * float3(sh[6], sh[7], sh[8]);
    result -= SH_C1 * x * float3(sh[9], sh[10], sh[11]);

    // Level 2
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, yz = y * z, xz = x * z;

    result += SH_C2[0] * xy * float3(sh[12], sh[13], sh[14]);
    result += SH_C2[1] * yz * float3(sh[15], sh[16], sh[17]);
    result += SH_C2[2] * (2.0f * zz - xx - yy) * float3(sh[18], sh[19], sh[20]);
    result += SH_C2[3] * xz * float3(sh[21], sh[22], sh[23]);
    result += SH_C2[4] * (xx - yy) * float3(sh[24], sh[25], sh[26]);

    // Level 3
    result += SH_C3[0] * y * (3.0f * xx - yy) * float3(sh[27], sh[28], sh[29]);
    result += SH_C3[1] * xy * z * float3(sh[30], sh[31], sh[32]);
    result += SH_C3[2] * y * (4.0f * zz - xx - yy) * float3(sh[33], sh[34], sh[35]);
    result += SH_C3[3] * z * (2.0f * zz - 3.0f * xx - 3.0f * yy) * float3(sh[36], sh[37], sh[38]);
    result += SH_C3[4] * x * (4.0f * zz - xx - yy) * float3(sh[39], sh[40], sh[41]);
    result += SH_C3[5] * z * (xx - yy) * float3(sh[42], sh[43], sh[44]);
    result += SH_C3[6] * x * (xx - 3.0f * yy) * float3(sh[45], sh[46], sh[47]);

    result += 0.5f;
    return result;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
<<<<<<< Updated upstream
    output.pos = float4(input.pos, 1.0f);
    output.color = input.color;
=======
    output.pos = mul(rotationMat, input.pos);
    output.color = float4(computeColorFromSH(input.pos.xyz), 1.0f);
>>>>>>> Stashed changes
    return output;
}
