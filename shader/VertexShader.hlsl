cbuffer ConstantBuffer : register(b0)
{
    float4x4 transformMat;
    
    float4x4 rotationMat;
    float4x4 projectionMat;
    float4x4 viewMat;
    float3 hfovxy_focal;
};

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float3 f_rest[16] : TEXCOORD0;
    float3 scale : TEXCOORD16;
    float4 rotation : TEXCOORD17;
    float opacity : TEXCOORD18;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float opacity : TEXCOORD18;
    float3 conic : TEXCOORD19; 
    float3 hfovxy_focal : TEXCOORD20;
    float2 coordxy : TEXCOORD21;
};

static const float SH_C0 = 0.2820947918f;
static const float SH_C1 = 0.4886025119f;
static const float SH_C2[5] = { 1.0925484306f, 1.0925484306f, 0.3153915653f, 1.0925484306f, 0.5462742153f };
static const float SH_C3[7] = { 0.5900435898f, 2.8906114426f, 0.4570457995f, 0.3731763325f, 0.4570457995f, 1.4453057213f, 0.5900435898f };

float3 computeColorFromSH(float3 position, float3 f_rest[16])
{
    float3 cam_pos = mul(transformMat, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
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

float3x3 computeCov3D(float3 scale, float4 q)
{
    float3x3 S = float3x3(
        scale.x, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f,
        0.0f, 0.0f, scale.z
    );

    float r = q.x;
    float x = q.y;
    float y = q.z;
    float z = q.w;

    float3x3 R = float3x3(
        1.0f - 2.0f * (y * y + z * z), 2.0f * (x * y - r * z), 2.0f * (x * z + r * y),
        2.0f * (x * y + r * z), 1.0f - 2.0f * (x * x + z * z), 2.0f * (y * z - r * x),
        2.0f * (x * z - r * y), 2.0f * (y * z + r * x), 1.0f - 2.0f * (x * x + y * y)
    );

    float3x3 M = mul(S, R);
    float3x3 Sigma = mul(transpose(M), M);
    return Sigma;
}

float3 computeCov2D(float4 mean_view, float focal_x, float focal_y, float tan_fovx, float tan_fovy, float3x3 cov3D, float4x4 viewmatrix)
{
    float4 t = mean_view;
    // why need this? Try remove this later
    float limx = 1.3f * tan_fovx;
    float limy = 1.3f * tan_fovy;
    float txtz = t.x / t.z;
    float tytz = t.y / t.z;
    t.x = min(limx, max(-limx, txtz)) * t.z;
    t.y = min(limy, max(-limy, tytz)) * t.z;

    float3x3 J = float3x3(
        focal_x / t.z, 0.0f, -(focal_x * t.x) / (t.z * t.z),
        0.0f, focal_y / t.z, -(focal_y * t.y) / (t.z * t.z),
        0.0f, 0.0f, 0.0f
    );
    float3x3 W = transpose((float3x3) viewmatrix);
    float3x3 T = mul(W, J);

    float3x3 cov = mul(transpose(T), mul(transpose(cov3D), T));
    cov[0][0] += 0.3f;
    cov[1][1] += 0.3f;
    return float3(cov[0][0], cov[0][1], cov[1][1]);
}


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(transformMat, input.pos);
    output.color = float4(computeColorFromSH(input.pos.xyz, input.f_rest), 1.0f);
    
    float4 pos_view = mul(viewMat, input.pos);
    float4 pos_screen = mul(projectionMat, pos_view);
    
    float3x3 cov3d = computeCov3D(input.scale, input.rotation);
    float2 wh = 2 * hfovxy_focal.xy * hfovxy_focal.z;
    float3 cov2d = computeCov2D(pos_view,
                              hfovxy_focal.z,
                              hfovxy_focal.z,
                              hfovxy_focal.x,
                              hfovxy_focal.y,
                              cov3d,
                              viewMat);

    float det = (cov2d.x * cov2d.z - cov2d.y * cov2d.y);
    
    float det_inv = 1.f / det;
    float3 conic = float3(cov2d.z * det_inv, -cov2d.y * det_inv, cov2d.x * det_inv);
    
    float2 quadwh_scr = float2(3.f * sqrt(cov2d.x), 3.f * sqrt(cov2d.z));
    float2 quadwh_ndc = quadwh_scr / wh * 2;
    pos_screen.xy = pos_screen.xy + input.pos.xy * quadwh_ndc;
    float2 coordxy = input.pos.xy * quadwh_scr;
    
    output.conic = conic;
    output.hfovxy_focal = hfovxy_focal;
    output.coordxy = coordxy;
    output.opacity = input.opacity;

    return output;
}

