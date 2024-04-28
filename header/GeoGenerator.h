#ifndef GEOGENERATOR_H
#define GEOGENERATOR_H

#include <vector>
#include <cstdint>
#include "Vertex.h"  // Ensure this path is correct for your Vertex structure

class GeoGenerator {
public:
    using uint16 = std::uint16_t;
    using uint32 = std::uint32_t;

    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<uint32> indices;

        std::vector<uint16> GetIndices16() const;
    };

    // Method to generate quads for each vertex
    std::vector<MeshData> GenerateQuadsForVertices(const std::vector<Vertex>& vertices, float quadSize);

private:
    // Helper method to create a single quad
    MeshData CreateQuad(const Vertex& baseVertex, float size);
};

#endif // GEOGENERATOR_H
