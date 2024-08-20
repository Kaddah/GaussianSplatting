#include "DepthSort.h"
#include <algorithm>

#include <glm/glm.hpp>
#include <vector>

// Function to calculate the depth of a vertex relative to the camera
float CalculateDepth(const glm::vec3& cameraPos, const Vertex& vertex)
{
  glm::vec3 toVertex = vertex.pos - cameraPos;
  return glm::dot(toVertex, toVertex); // squared distance to avoid sqrt
}

void BitonicCompareAndSwap(std::vector<std::pair<float, uint32_t>>& arr, int i, int j, bool ascending)
{
  if ((ascending && arr[i].first > arr[j].first) || (!ascending && arr[i].first < arr[j].first))
  {
    std::swap(arr[i], arr[j]);
  }
}

void BitonicMerge(std::vector<std::pair<float, uint32_t>>& arr, int low, int count, bool ascending)
{
  if (count > 1)
  {
    int mid = count / 2;
    for (int i = low; i < low + mid; i++)
    {
      BitonicCompareAndSwap(arr, i, i + mid, ascending);
    }
    BitonicMerge(arr, low, mid, ascending);
    BitonicMerge(arr, low + mid, mid, ascending);
  }
}

void BitonicSortUtil(std::vector<std::pair<float, uint32_t>>& arr, int low, int count, bool ascending)
{
  if (count > 1)
  {
    int mid = count / 2;

    // Sort in ascending order since we're doing a full bitonic sort
    BitonicSortUtil(arr, low, mid, true);

    // Sort in descending order
    BitonicSortUtil(arr, low + mid, mid, false);

    // Merge the entire sequence
    BitonicMerge(arr, low, count, ascending);
  }
}

// Function to sort the indices based on depth using Bitonic Sort
void SortIndicesByDepth(const glm::vec3& cameraPos, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
  std::vector<std::pair<float, uint32_t>> depthIndexPairs;

  for (size_t i = 0; i < indices.size(); ++i)
  {
    float depth = CalculateDepth(cameraPos, vertices[indices[i]]);
    depthIndexPairs.emplace_back(depth, indices[i]);
  }

  // Ensure that the number of elements is a power of two for bitonic sort
  int n = depthIndexPairs.size();
  int p = 1;
  while (p < n)
    p <<= 1; // Find the next power of two greater or equal to n
  depthIndexPairs.resize(p, {-std::numeric_limits<float>::infinity(), 0}); // Fill with negative infinity depth

  // Perform bitonic sort
  BitonicSortUtil(depthIndexPairs, 0, p, true);

  // Remove the padding and copy the sorted indices back
  indices.resize(n);
  for (size_t i = 0; i < n; ++i)
  {
    indices[i] = depthIndexPairs[i].second;
  }
  // test invert indexarray far to near
  std::reverse(indices.begin(), indices.end());
}