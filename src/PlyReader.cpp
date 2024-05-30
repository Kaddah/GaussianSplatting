//#include "PlyReader.h"
//#include <iostream>
//#include <fstream>
//#include <sstream>
//
//std::vector<Vertex> PlyReader::readPlyFile(const std::string& filename) {
//    std::ifstream plyFile(filename, std::ios::binary);
//    if (!plyFile.is_open()) {
//        std::cerr << "Konnte die PLY-Datei nicht öffnen." << std::endl;
//        return std::vector<Vertex>();
//    }
//
//    std::string line;
//    while (std::getline(plyFile, line)) {
//        // Check and remove carriage return character at the end of the line if present
//        if (!line.empty() && line.back() == '\r') {
//            line.pop_back();
//        }
//        if (line == "end_header") {
//            break;
//        }
//    }
//
//    std::vector<Vertex> vertices;
//
//    Vertex vertex;
//    int nx, ny, nz, nbla;
//
//    while (std::getline(plyFile, line)) {
//        std::istringstream iss(line);
//        Vertex vertex;
//        // Hier kommen Daten direkt in die Struktur
//        if (!(iss >> vertex.pos.x >> vertex.pos.y >> vertex.pos.z >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z))
//        {
//            std::cerr << "Fehler beim Parsen der PLY-Datei." << std::endl;
//            break;
//        }
//        // Check if there are 3 or 4 color components
//        if (iss >> vertex.color.r >> vertex.color.g >> vertex.color.b)
//        {
//            // Convert colors from unsigned char to the range [0, 1]
//            vertex.color /= 255.0f;
//            // Set alpha to 1.0f as default
//            vertex.color.a = 1.0f;
//        }
//        else if (iss >> vertex.color.r >> vertex.color.g >> vertex.color.b >> vertex.color.a)
//        {
//            // Convert colors from unsigned char to the range [0, 1]
//            vertex.color /= 255.0f;
//        }
//        else
//        {
//            std::cerr << "Fehler beim Lesen der Farben aus der PLY-Datei." << std::endl;
//            break;
//        }
//        // Vertex zur Liste hinzufügen
//        vertices.push_back(vertex);
//    }


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

    //#######
    //plyFile.close();

    //return vertices;
//}


#include "PlyReader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

std::vector<Vertex> PlyReader::readPlyFile(const std::string& filename)
{
    std::ifstream plyFile(filename, std::ios::binary);
    if (!plyFile.is_open())
    {
        std::cerr << "Konnte die PLY-Datei nicht öffnen." << std::endl;
        return std::vector<Vertex>();
    }

    std::string                          line;
    std::string                          format;
    std::vector<std::string>             properties;
    std::unordered_map<std::string, int> propertyIndex;
    bool                                 isBinary = false;

    // Read header
    while (std::getline(plyFile, line))
    {
        // Check and remove carriage return character at the end of the line if present
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.substr(0, 7) == "format ")
        {
            format = line.substr(7);
            if (format.find("binary") != std::string::npos)
            {
              isBinary = true;
            }
        }
        if (line.substr(0, 8) == "property")
        {
            std::istringstream iss(line);
            std::string        propertyType, propertyName;
            iss >> propertyType >> propertyType >> propertyName;
            properties.push_back(propertyName);
            propertyIndex[propertyName] = properties.size() - 1;
        }
        if (line == "end_header")
        {
            break;
        }
    }

    std::vector<Vertex> vertices;

    if (isBinary)
    {
        plyFile.close();
        plyFile.open(filename, std::ios::binary);
        if (!plyFile.is_open())
        {
            std::cerr << "Konnte die PLY-Datei nicht erneut öffnen." << std::endl;
            return std::vector<Vertex>();
        }

        // Skip header
        while (std::getline(plyFile, line))
        {
            if (line == "end_header")
            {
              break;
            }
        }

        // Read binary data
        while (!plyFile.eof())
        {
            Vertex vertex;
            for (const auto& property : properties)
            {
              if (property == "x")
                plyFile.read(reinterpret_cast<char*>(&vertex.pos.x), sizeof(float));
              else if (property == "y")
                plyFile.read(reinterpret_cast<char*>(&vertex.pos.y), sizeof(float));
              else if (property == "z")
                plyFile.read(reinterpret_cast<char*>(&vertex.pos.z), sizeof(float));
              else if (property == "nx")
                plyFile.read(reinterpret_cast<char*>(&vertex.normal.x), sizeof(float));
              else if (property == "ny")
                plyFile.read(reinterpret_cast<char*>(&vertex.normal.y), sizeof(float));
              else if (property == "nz")
                plyFile.read(reinterpret_cast<char*>(&vertex.normal.z), sizeof(float));
              else if (property == "red")
                plyFile.read(reinterpret_cast<char*>(&vertex.color.r), sizeof(unsigned char));
              else if (property == "green")
                plyFile.read(reinterpret_cast<char*>(&vertex.color.g), sizeof(unsigned char));
              else if (property == "blue")
                plyFile.read(reinterpret_cast<char*>(&vertex.color.b), sizeof(unsigned char));
              else if (property == "alpha")
                plyFile.read(reinterpret_cast<char*>(&vertex.color.a), sizeof(unsigned char));
            }
            // Convert colors from unsigned char to the range [0, 1]
            vertex.color /= 255.0f;
            vertices.push_back(vertex);
        }
    }
    else
    {
        // Read ASCII data
        while (std::getline(plyFile, line))
        {
            std::istringstream iss(line);
            Vertex             vertex;
            for (const auto& property : properties)
            {
              if (property == "x")
                iss >> vertex.pos.x;
              else if (property == "y")
                iss >> vertex.pos.y;
              else if (property == "z")
                iss >> vertex.pos.z;
              else if (property == "nx")
                iss >> vertex.normal.x;
              else if (property == "ny")
                iss >> vertex.normal.y;
              else if (property == "nz")
                iss >> vertex.normal.z;
              else if (property == "red")
                iss >> vertex.color.r;
              else if (property == "green")
                iss >> vertex.color.g;
              else if (property == "blue")
                iss >> vertex.color.b;
              else if (property == "alpha")
                iss >> vertex.color.a;
            }
            // Convert colors from unsigned char to the range [0, 1]
            vertex.color /= 255.0f;
            vertices.push_back(vertex);
        }
    }

    plyFile.close();

    return vertices;
}
