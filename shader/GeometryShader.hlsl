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
    // Number of segments for Gaussian splatting
    int numSegments = 24; // Increase for more detail
    float angleStep = 2.0 * 3.14159265359 / numSegments;

    // Standard deviation for Gaussian distribution
    float sigma = 0.1; // Adjust to change "splat" size default=0.1

    // Iterate over each input triangle
    for (int i = 0; i < 3; ++i)
    {
        // Central vertex for the filled circle (splat)
        GS_OUTPUT centerOutput;
        centerOutput.pos = input[i].pos;
        centerOutput.color = input[i].color; // Use the color from input vertex
        centerOutput.offset = float2(0.0, 0.0);

        // Generate triangles for the circle around each vertex
        for (int j = 0; j < numSegments; ++j)
        {
            float angle0 = j * angleStep;
            float angle1 = (j + 1) * angleStep;
            float2 offset0 = float2(cos(angle0), sin(angle0)) * sigma;
            float2 offset1 = float2(cos(angle1), sin(angle1)) * sigma;

            GS_OUTPUT output0, output1;

            output0.pos = input[i].pos + float4(offset0.x, offset0.y, 0.0f, 0.0f);
            output0.color = input[i].color; // Use the color from input vertex
            output0.offset = offset0;

            output1.pos = input[i].pos + float4(offset1.x, offset1.y, 0.0f, 0.0f);
            output1.color = input[i].color; // Use the color from input vertex
            output1.offset = offset1;

            // Emit the triangle (center, output0, output1)
            outputStream.Append(centerOutput);
            outputStream.Append(output0);
            outputStream.Append(output1);
        }
    }
}
