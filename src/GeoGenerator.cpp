
#include "GeoGenerator.h"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

// Convert uint32 indices to uint16 for some graphics APIs that require it
std::vector<GeoGenerator::uint16> GeoGenerator::MeshData::GetIndices16() const
{
  std::vector<uint16> indices16;
  /*   indices16.reserve(indices.size());
     for (auto index : indices) {
         indices16.push_back(static_cast<uint16>(index));
     }*/
  return indices16;
}

// Create a quad around a given vertex
GeoGenerator::MeshData GeoGenerator::CreateQuad(const Vertex& baseVertex, float size)
{
  MeshData meshData;

  // First triangle (p0, p1, p2)
  // meshData.vertices.push_back(Vertex{ baseVertex.pos, baseVertex.normal, baseVertex.color });

  // Print the vertices
  // std::cout << "Vertices:\n";
  // for (const auto& vertex : meshData.vertices)
  //{
  //     std::cout << "Position: (" << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << ")\n";
  //     std::cout << "Normal: (" << vertex.normal.x << ", " << vertex.normal.y << ", " << vertex.normal.z << ")\n";
  //     std::cout << "Color: (" << vertex.color.r << ", " << vertex.color.g << ", " << vertex.color.b  << ")\n";
  //     std::cout << "\n";
  // }

  // No indices are needed as we're now using direct vertex order for rendering
  return meshData;
}

// Generate a quad for each vertex in the vector
std::vector<GeoGenerator::MeshData> GeoGenerator::GenerateQuadsForVertices(const std::vector<Vertex>& vertices,
                                                                           float                      quadSize)
{
  std::vector<GeoGenerator::MeshData> quads;
  // quads.reserve(vertices.size());  // Preallocate memory for efficiency

  // for (const Vertex& vertex : vertices) {
  //     MeshData quad = CreateQuad(vertex, quadSize);
  //     quads.push_back(quad);
  // }

  return quads;
};