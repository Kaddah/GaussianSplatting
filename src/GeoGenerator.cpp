
#include "GeoGenerator.h"
#include <vector>
#include <glm/glm.hpp>

// Convert uint32 indices to uint16 for some graphics APIs that require it
std::vector<GeoGenerator::uint16> GeoGenerator::MeshData::GetIndices16() const {
    std::vector<uint16> indices16;
    indices16.reserve(indices.size());
    for (auto index : indices) {
        indices16.push_back(static_cast<uint16>(index));
    }
    return indices16;
}

// Create a quad around a given vertex
GeoGenerator::MeshData GeoGenerator::CreateQuad(const Vertex& baseVertex, float size) {
    MeshData meshData;
    float halfSize = size / 2.0f;

    // Calculate the quad vertices around the base vertex position
    glm::vec3 p0 = baseVertex.pos + glm::vec3(-halfSize, -halfSize, 0);
    glm::vec3 p1 = baseVertex.pos + glm::vec3(halfSize, -halfSize, 0);
    glm::vec3 p2 = baseVertex.pos + glm::vec3(halfSize, halfSize, 0);
    glm::vec3 p3 = baseVertex.pos + glm::vec3(-halfSize, halfSize, 0);

    meshData.vertices.push_back(Vertex{ p0, baseVertex.normal, baseVertex.color });
    meshData.vertices.push_back(Vertex{ p1, baseVertex.normal, baseVertex.color });
    meshData.vertices.push_back(Vertex{ p2, baseVertex.normal, baseVertex.color });
    meshData.vertices.push_back(Vertex{ p3, baseVertex.normal, baseVertex.color });

    // Create two triangles from vertices
    uint32 baseIndex = static_cast<uint32>(meshData.vertices.size() - 4);
    meshData.indices.push_back(baseIndex);
    meshData.indices.push_back(baseIndex + 1);
    meshData.indices.push_back(baseIndex + 2);
    meshData.indices.push_back(baseIndex);
    meshData.indices.push_back(baseIndex + 2);
    meshData.indices.push_back(baseIndex + 3);

    return meshData;
}

// Generate a quad for each vertex in the vector
std::vector <GeoGenerator::MeshData> GeoGenerator::GenerateQuadsForVertices(const std::vector<Vertex>& vertices, float quadSize) {
    std::vector<GeoGenerator::MeshData> quads;
    quads.reserve(vertices.size());  // Preallocate memory for efficiency

    for (const Vertex& vertex : vertices) {
        MeshData quad = CreateQuad(vertex, quadSize);
        quads.push_back(quad);
    }

    return quads;
};

