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
    int nx, ny, nz, nbla;

    while (std::getline(plyFile, line)) {
        std::istringstream iss(line);
        Vertex vertex;
        // Hier kommen Daten direkt in die Struktur
        if (!(iss >> vertex.pos.x >> vertex.pos.y >> vertex.pos.z >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z))
        {
            std::cerr << "Fehler beim Parsen der PLY-Datei." << std::endl;
            break;
        }
        // Check if there are 3 or 4 color components
        if (iss >> vertex.color.r >> vertex.color.g >> vertex.color.b)
        {
            // Convert colors from unsigned char to the range [0, 1]
            vertex.color /= 255.0f;
            // Set alpha to 1.0f as default
            vertex.color.a = 1.0f;
        }
        else if (iss >> vertex.color.r >> vertex.color.g >> vertex.color.b >> vertex.color.a)
        {
            // Convert colors from unsigned char to the range [0, 1]
            vertex.color /= 255.0f;
        }
        else
        {
            std::cerr << "Fehler beim Lesen der Farben aus der PLY-Datei." << std::endl;
            break;
        }
        // Vertex zur Liste hinzufügen
        vertices.push_back(vertex);
    }


    //while (std::getline(plyFile, line))
    //{
    //    std::istringstream iss(line);
    //    Vertex             vertex;
    //    // Read position
    //    if (!(iss >> vertex.pos.x >> vertex.pos.y >> vertex.pos.z))
    //    {
    //        std::cerr << "Fehler beim Parsen der PLY-Datei (Position)." << std::endl;
    //        break;
    //    }
    //    // Read color (RGBA)
    //    if (!(iss >> vertex.color.r >> vertex.color.g >> vertex.color.b >> vertex.color.a))
    //    {
    //        std::cerr << "Fehler beim Parsen der PLY-Datei (Farbe)." << std::endl;
    //        break;
    //    }
    //    // Read normal
    //    if (!(iss >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z))
    //    {
    //        std::cerr << "Fehler beim Parsen der PLY-Datei (Normal)." << std::endl;
    //        break;
    //    }


    //    // Convert colors from unsigned char to the range [0, 1]
    //    vertex.color /= 255.0f;
    //    // Vertex zur Liste hinzufügen
    //    vertices.push_back(vertex);
    //}


    plyFile.close();

    return vertices;
}
