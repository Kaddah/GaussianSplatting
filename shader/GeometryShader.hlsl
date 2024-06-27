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

static const int numTriangles = 4;

[maxvertexcount(numTriangles * 3)] void main(point GS_INPUT input[1], inout TriangleStream<GS_OUTPUT> outputStream)
{
  // Number of segments for Gaussian splatting
  float angleStep = 2.0 * 3.14159265359 / numTriangles;

  // Standard deviation for Gaussian distribution
  float sigma = 0.1; // Adjust to change "splat" size default=0.1

  // Iterate over each input triangle

  // Central vertex for the filled circle (splat)
  GS_OUTPUT centerOutput;
  centerOutput.pos    = input[0].pos;
  centerOutput.color  = input[0].color; // Use the color from input vertex
  centerOutput.offset = float2(0.0, 0.0);

  // Generate triangles for the circle around each vertex
  for (int j = 0; j < numTriangles; ++j)
  {
    float  angle0  = j * angleStep;
    float  angle1  = (j + 1) * angleStep;
    float2 offset0 = float2(cos(angle0), sin(angle0)) * sigma;
    float2 offset1 = float2(cos(angle1), sin(angle1)) * sigma;

    GS_OUTPUT output0, output1;

    output0.pos    = input[0].pos + float4(offset0.x, offset0.y, 0.0f, 0.0f);
    output0.color  = input[0].color; // Use the color from input vertex
    output0.offset = offset0;

    output1.pos    = input[0].pos + float4(offset1.x, offset1.y, 0.0f, 0.0f);
    output1.color  = input[0].color; // Use the color from input vertex
    output1.offset = offset1;

    // Emit the triangle (center, output0, output1)
    outputStream.Append(centerOutput);
    outputStream.Append(output0);
    outputStream.Append(output1);
    outputStream.RestartStrip();
  }
}