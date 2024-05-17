struct GS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

struct GS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 offset : OFFSET; // Offset for Gaussian splatting
};

[maxvertexcount(72)]
void main(triangle GS_INPUT input[3], inout TriangleStream<GS_OUTPUT> outputStream)
{
    // Calculate the center of the triangle
    float4 center = (input[0].pos + input[1].pos + input[2].pos) / 3.0;
    float4 color = (input[0].color + input[1].color + input[2].color) / 3.0;

    // Number of segments for Gaussian splatting
    int numSegments = 24; // Increase for more detail
    float angleStep = 2.0 * 3.14159265359 / numSegments;

    // Standard deviation for Gaussian distribution
    float sigma = 0.05; // Adjust to change "splat" size default=0.1

    // Generate points for Gaussian splatting
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < numSegments; ++j)
        {
            float angle = j * angleStep;
            float2 offset = float2(cos(angle), sin(angle)) * sigma;

            GS_OUTPUT output;
            output.pos = input[i].pos + float4(offset.x, offset.y, 0.0f, 0.0f); // Correct position using triangle center
            output.color = input[i].color;
            output.offset = offset;
            outputStream.Append(output);
        }
    }

    outputStream.RestartStrip();
}
