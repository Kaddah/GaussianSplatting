#include "DepthSort.h"
#include <algorithm>

// Function to calculate the depth of a vertex relative to the camera
float CalculateDepth(const glm::vec3& cameraPos, const Vertex& vertex)
{
  glm::vec3 toVertex = vertex.pos - cameraPos;
  return glm::dot(toVertex, toVertex); // squared distance to avoid sqrt
}

// Function to sort the indices based on depth
void SortIndicesByDepth(const glm::vec3& cameraPos, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
  std::vector<std::pair<float, uint32_t>> depthIndexPairs;

  for (size_t i = 0; i < indices.size(); ++i)
  {
    float depth = CalculateDepth(cameraPos, vertices[indices[i]]);
    depthIndexPairs.emplace_back(depth, indices[i]);
  }

  std::sort(depthIndexPairs.begin(), depthIndexPairs.end(),
            [](const std::pair<float, uint32_t>& a, const std::pair<float, uint32_t>& b)
            {
              return a.first > b.first; // sort in descending order
            });

  for (size_t i = 0; i < depthIndexPairs.size(); ++i)
  {
    indices[i] = depthIndexPairs[i].second;
  }
}
