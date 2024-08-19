struct GS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float opacity : TEXCOORD18;
    float3 conic : TEXCOORD19;
    float3 hfovxy_focal : TEXCOORD20;
    float2 coordxy : TEXCOORD21;
};

struct GS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 offset : OFFSET;
    float opacity : TEXCOORD18;
    float3 conic : TEXCOORD19;
    float3 hfovxy_focal : TEXCOORD20;
    float2 coordxy : TEXCOORD21;
};

static const int numTriangles = 16;

[maxvertexcount(numTriangles * 3)]void main(point GS_INPUT input[1], inout TriangleStream<GS_OUTPUT> outputStream)
{
    float angleStep = 2.0 * 3.14159265359 / numTriangles;
    float sigma = 0.1;

    GS_OUTPUT centerOutput;
    centerOutput.pos = input[0].pos;
    centerOutput.color = input[0].color;
    centerOutput.offset = float2(0.0, 0.0);
    centerOutput.opacity = input[0].opacity;
    centerOutput.conic = input[0].conic;
    centerOutput.hfovxy_focal = input[0].hfovxy_focal;
    centerOutput.coordxy = input[0].coordxy;

    for (int j = 0; j < numTriangles; ++j)
    {
        float angle0 = j * angleStep;
        float angle1 = (j + 1) * angleStep;
        float2 offset0 = float2(cos(angle0), sin(angle0)) * sigma;
        float2 offset1 = float2(cos(angle1), sin(angle1)) * sigma;

        GS_OUTPUT output0, output1;

        output0.pos = input[0].pos + float4(offset0.x, offset0.y, 0.0f, 0.0f);
        output0.color = input[0].color;
        output0.offset = offset0;
        output0.opacity = input[0].opacity;
        output0.conic = input[0].conic;
        output0.hfovxy_focal = input[0].hfovxy_focal;
        output0.coordxy = input[0].coordxy;

        output1.pos = input[0].pos + float4(offset1.x, offset1.y, 0.0f, 0.0f);
        output1.color = input[0].color;
        output1.offset = offset1;
        output1.opacity = input[0].opacity;
        output1.conic = input[0].conic;
        output1.hfovxy_focal = input[0].hfovxy_focal;
        output1.coordxy = input[0].coordxy;

        outputStream.Append(centerOutput);
        outputStream.Append(output0);
        outputStream.Append(output1);
        outputStream.RestartStrip();
    }
}
