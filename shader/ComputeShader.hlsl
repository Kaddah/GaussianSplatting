// ComputeShader.hlsl
struct VertexPos
{
    float3 pos;
    float alpha;
};

cbuffer Constants : register(b0)
{
    float4x4 viewProj;
    uint numVertices;
};

StructuredBuffer<VertexPos> inputPositions : register(t0);
RWStructuredBuffer<VertexPos> outputPositions : register(u0);

groupshared VertexPos sharedPositions[256];

void BitonicSort(uint tid, uint k, uint j)
{
    uint ixj = tid ^ j;
    if (ixj > tid)
    {
        float depth1 = mul(viewProj, float4(sharedPositions[tid].pos, 1.0)).z;
        float depth2 = mul(viewProj, float4(sharedPositions[ixj].pos, 1.0)).z;

        bool swap = (depth1 > depth2) == ((tid & k) == 0);

        if (swap)
        {
            VertexPos temp = sharedPositions[tid];
            sharedPositions[tid] = sharedPositions[ixj];
            sharedPositions[ixj] = temp;
        }
    }
}

[numthreads(256, 1, 1)]void main(uint3 DTid : SV_DispatchThreadID)
{
    uint tid = DTid.x;
    uint lid = DTid.x % 256;

    if (tid < numVertices)
    {
        // Load data into shared memory
        sharedPositions[lid] = inputPositions[tid];

        // Example: Set alpha based on depth (distance to camera)
        float4 posWorld = float4(sharedPositions[lid].pos, 1.0f);
        float4 posView = mul(viewProj, posWorld);
        float depth = posView.z / posView.w; // Normalize by w
        sharedPositions[lid].alpha = saturate(1.0f - depth); // Example: Decrease alpha with depth
    }

    GroupMemoryBarrierWithGroupSync();

    // Bitonic sort within the group
    for (uint k = 2; k <= 256; k <<= 1)
    {
        for (uint j = k >> 1; j > 0; j >>= 1)
        {
            BitonicSort(lid, k, j);
            GroupMemoryBarrierWithGroupSync();
        }
    }

    // Write sorted and alpha-modified data to output buffer
    if (tid < numVertices)
    {
        outputPositions[tid] = sharedPositions[lid];
    }
}
