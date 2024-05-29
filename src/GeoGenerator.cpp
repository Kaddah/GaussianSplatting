
#include "GeoGenerator.h"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>

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
    float halfSize = size / 8.0f;

    // Calculate the quad vertices around the base vertex position
    glm::vec3 p0 = baseVertex.pos + glm::vec3(-halfSize, -halfSize, 0);
    glm::vec3 p1 = baseVertex.pos + glm::vec3(halfSize, -halfSize, 0);
    glm::vec3 p2 = baseVertex.pos + glm::vec3(halfSize, halfSize, 0);
    glm::vec3 p3 = baseVertex.pos + glm::vec3(-halfSize, halfSize, 0);

    // First triangle (p0, p1, p2)
    meshData.vertices.push_back(Vertex{ p0, baseVertex.normal, baseVertex.color });
    meshData.vertices.push_back(Vertex{ p1, baseVertex.normal, baseVertex.color });
    meshData.vertices.push_back(Vertex{ p2, baseVertex.normal, baseVertex.color });

    // Second triangle (p2, p3, p0)
    meshData.vertices.push_back(Vertex{ p2, baseVertex.normal, baseVertex.color });
    meshData.vertices.push_back(Vertex{ p3, baseVertex.normal, baseVertex.color });
    meshData.vertices.push_back(Vertex{ p0, baseVertex.normal, baseVertex.color });


        // Print the vertices
    //std::cout << "Vertices:\n";
    //for (const auto& vertex : meshData.vertices)
    //{
    //    std::cout << "Position: (" << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << ")\n";
    //    std::cout << "Normal: (" << vertex.normal.x << ", " << vertex.normal.y << ", " << vertex.normal.z << ")\n";
    //    std::cout << "Color: (" << vertex.color.r << ", " << vertex.color.g << ", " << vertex.color.b  << ")\n";
    //    std::cout << "\n";
    //}

    // No indices are needed as we're now using direct vertex order for rendering
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