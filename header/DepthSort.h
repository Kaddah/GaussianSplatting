#ifndef DEPTHSORT_H
#define DEPTHSORT_H

#include "Vertex.h" // Include this if you have a separate file for the Vertex struct, otherwise use appropriate include
#include <glm/glm.hpp>
#include <vector>

// Function to calculate the depth of a vertex relative to the camera
float CalculateDepth(const glm::vec3& cameraPos, const Vertex& vertex);

// Function to sort the indices based on depth
void SortIndicesByDepth(const glm::vec3& cameraPos, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

#endif // DEPTHSORT_H
