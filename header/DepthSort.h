#ifndef DEPTHSORT_H
#define DEPTHSORT_H

#include "Vertex.h" // Include this if you have a separate file for the Vertex struct, otherwise use appropriate include
#include <glm/glm.hpp>
#include <vector>

// Funktion zur Berechnung der Tiefe eines Vertex relativ zur Kamera
float CalculateDepth(const glm::vec3& cameraPos, const Vertex& vertex);

// Funktion, die Bitonic Compare and Swap implementiert
void BitonicCompareAndSwap(std::vector<std::pair<float, uint32_t>>& arr, int i, int j, bool ascending);

// Funktion, die Bitonic Merge implementiert
void BitonicMerge(std::vector<std::pair<float, uint32_t>>& arr, int low, int count, bool ascending);

// Rekursive Hilfsfunktion für Bitonic Sort
void BitonicSortUtil(std::vector<std::pair<float, uint32_t>>& arr, int low, int count, bool ascending);

// Funktion, die die Indizes der Vertices basierend auf der Tiefe sortiert
void SortIndicesByDepth(const glm::vec3& cameraPos, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

#endif // DEPTHSORT_H
