cbuffer ConstantBuffer : register(b0)
{
    float4x4 transformMat;
    
    float4x4 rotationMat;
    float4x4 projectionMat;
    float4x4 viewMat;
    float3 hfovxy_focal;
};

struct GS_INPUT
{
    float4 pos : SV_POSITION;    
    float3 color : COLOR0;
    float alpha : COLOR1;
    float3 cov2d : COLOR2;
};

struct GS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float alpha : COLOR1;
    float3 conic : COLOR2;
    float2 coordxy : COLOR3;
};

static const int numTriangles = 2;

[maxvertexcount(4)] void main(point GS_INPUT input[1], inout TriangleStream<GS_OUTPUT> outputStream){

    float det = (input[0].cov2d.x * input[0].cov2d.z - input[0].cov2d.y * input[0].cov2d.y);
    float det_inv = 1.f / det;
    
    float3 conic = det_inv * float3(input[0].cov2d.z, -input[0].cov2d.y, input[0].cov2d.x);
    
    float2 quadwh_scr = float2(3.f * sqrt(input[0].cov2d.x), 3.f * sqrt(input[0].cov2d.z));
    
    float2 wh = 2 * hfovxy_focal.xy * hfovxy_focal.z;
    float2 quadwh_ndc = quadwh_scr / wh * 2;
    
    quadwh_ndc = float2(0.1, 0.1); // TODO
    
    GS_OUTPUT output;
    output.color = input[0].color.rgb;
    output.alpha = input[0].alpha;
    output.conic = conic;
        
    float2 pos;
    
    pos = float2(-1, 1);
    output.pos     = input[0].pos + float4(pos * quadwh_ndc, 0, 0);
    output.coordxy = pos * quadwh_scr;    
    outputStream.Append(output);
    
    pos = float2(1, 1);
    output.pos     = input[0].pos + float4(pos * quadwh_ndc, 0, 0);
    output.coordxy = pos * quadwh_scr;    
    outputStream.Append(output);    
    
    pos = float2(-1, -1);
    output.pos     = input[0].pos + float4(pos * quadwh_ndc, 0, 0);
    output.coordxy = pos * quadwh_scr;    
    outputStream.Append(output);
    
    pos = float2(1, -1);
    output.pos     = input[0].pos + float4(pos * quadwh_ndc, 0, 0);
    output.coordxy = pos * quadwh_scr;    
    outputStream.Append(output);
}
