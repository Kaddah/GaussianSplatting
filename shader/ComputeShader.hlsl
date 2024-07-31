// ComputeShader.hlsl
struct VertexPos
{
    float3 pos;
    uint index;
};

cbuffer Constants : register(b0)
{
    float4x4 viewProj;
    uint numVertices;
};

StructuredBuffer<VertexPos> inputPositions : register(t0);
RWStructuredBuffer<VertexPos> outputPositions : register(u0);

groupshared VertexPos sharedPositions[256];

// Funktion zur Bitonic-Sortierung
void BitonicSort(uint tid, uint k, uint j)
{
    uint ixj = tid ^ j;
    if (ixj > tid)
    {
        // Berechnung der Tiefe zur Sortierung
        float depth1 = mul(viewProj, float4(sharedPositions[tid].pos, 1.0)).z;
        float depth2 = mul(viewProj, float4(sharedPositions[ixj].pos, 1.0)).z;

        // Entscheidung, ob getauscht werden soll
        bool swap = (depth1 > depth2) == ((tid & k) == 0);

        if (swap)
        {
            VertexPos temp = sharedPositions[tid];
            sharedPositions[tid] = sharedPositions[ixj];
            sharedPositions[ixj] = temp;
        }
    }
}

[numthreads(256, 1, 1)] // 256 Threads pro Gruppe
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint tid = DTid.x; // Globale Thread-ID
    uint lid = DTid.x % 256; // Lokale ID innerhalb der Gruppe

    if (tid < numVertices)
    {
        // Daten in den shared Speicher laden
        sharedPositions[lid] = inputPositions[tid];
    }

    // Synchronisierung der Gruppe
    GroupMemoryBarrierWithGroupSync();

    // Bitonic Sort innerhalb der Gruppe
    for (uint k = 2; k <= 256; k <<= 1)
    {
        for (uint j = k >> 1; j > 0; j >>= 1)
        {
            BitonicSort(lid, k, j);
            GroupMemoryBarrierWithGroupSync(); // Synchronisieren nach jedem Sortierschritt
        }
    }

    // Sortierte Daten in den Output Buffer schreiben
    if (tid < numVertices)
    {
        outputPositions[tid] = sharedPositions[lid];
    }
}
