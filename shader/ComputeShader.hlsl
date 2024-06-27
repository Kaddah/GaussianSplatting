// ComputeShader.hlsl
[numthreads(1, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    // Input buffer
    StructuredBuffer<float> inputBuffer : register(t0);
    
    // Output buffer
    RWStructuredBuffer<float> outputBuffer : register(u0);

    // Add 1 to each element
    outputBuffer[DTid.x] = inputBuffer[DTid.x] + 1.0f;
}
