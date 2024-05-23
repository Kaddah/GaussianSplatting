#include "PlyReader.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::vector<Vertex> PlyReader::readPlyFile(const std::string& filename) {
    std::ifstream plyFile(filename, std::ios::binary);
    if (!plyFile.is_open()) {
        std::cerr << "Konnte die PLY-Datei nicht öffnen." << std::endl;
        return std::vector<Vertex>();
    }

    std::string line;
    while (std::getline(plyFile, line)) {
        // Check and remove carriage return character at the end of the line if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line == "end_header") {
            break;
        }
    }

    std::vector<Vertex> vertices;

    Vertex vertex;

    while (std::getline(plyFile, line)) {
        std::istringstream iss(line);
        Vertex vertex;
        // Hier liest du die Daten direkt in die Struktur
        if (!(iss >> vertex.pos.x >> vertex.pos.y >> vertex.pos.z >>
            vertex.normal.x >> vertex.normal.y >> vertex.normal.z >>
            vertex.color.x >> vertex.color.y >> vertex.color.z)) {
            std::cerr << "Fehler beim Parsen der PLY-Datei." << std::endl;
            break;
        }
        // Konvertiere die Farben von unsigned char in den Bereich [0, 1]
        vertex.color /= 255.0f;
        // Vertex zur Liste hinzufügen
        vertices.push_back(vertex);
    }

    plyFile.close();

    return vertices;
}
