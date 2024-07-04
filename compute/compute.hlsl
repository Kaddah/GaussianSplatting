// compute.hlsl

RWStructuredBuffer<float2> g_outputBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // Beispieloperation: Kopiere Daten von Input in Output
    g_outputBuffer[DTid.x].x = 1.0f;
    g_outputBuffer[DTid.x].y = 1.0f;
}
