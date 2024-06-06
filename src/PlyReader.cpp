
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
          Vertex vertex = {};
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
            else if (property == "opacity")
              plyFile.read(reinterpret_cast<char*>(&vertex.opacity), sizeof(float));
            else if (property == "scale_0")
              plyFile.read(reinterpret_cast<char*>(&vertex.scale.x), sizeof(float));
            else if (property == "scale_1")
              plyFile.read(reinterpret_cast<char*>(&vertex.scale.y), sizeof(float));
            else if (property == "scale_2")
              plyFile.read(reinterpret_cast<char*>(&vertex.scale.z), sizeof(float));
            else if (property == "rot_0")
              plyFile.read(reinterpret_cast<char*>(&vertex.rotation.x), sizeof(float));
            else if (property == "rot_1")
              plyFile.read(reinterpret_cast<char*>(&vertex.rotation.y), sizeof(float));
            else if (property == "rot_2")
              plyFile.read(reinterpret_cast<char*>(&vertex.rotation.z), sizeof(float));
            else if (property == "rot_3")
              plyFile.read(reinterpret_cast<char*>(&vertex.rotation.w), sizeof(float));
            else if (property.substr(0, 7) == "f_rest_")
            {
              int index = std::stoi(property.substr(7));
              if (index >= 0 && index < 48)
              {
                plyFile.read(reinterpret_cast<char*>(&vertex.f_rest[index]), sizeof(float));
              }
            }
          }
          // Convert colors from unsigned char to the range [0, 1]
          vertex.color.r /= 255.0f;
          vertex.color.g /= 255.0f;
          vertex.color.b /= 255.0f;
          vertex.color.a /= 255.0f;
          vertices.push_back(vertex);
        }
    }
    else
    {
        // Read ASCII data
        while (std::getline(plyFile, line))
        {
          std::istringstream iss(line);
          Vertex             vertex = {};
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
            else if (property == "opacity")
              iss >> vertex.opacity;
            else if (property == "scale_0")
              iss >> vertex.scale.x;
            else if (property == "scale_1")
              iss >> vertex.scale.y;
            else if (property == "scale_2")
              iss >> vertex.scale.z;
            else if (property == "rot_0")
              iss >> vertex.rotation.x;
            else if (property == "rot_1")
              iss >> vertex.rotation.y;
            else if (property == "rot_2")
              iss >> vertex.rotation.z;
            else if (property == "rot_3")
              iss >> vertex.rotation.w;
            else if (property.substr(0, 7) == "f_rest_")
            {
              int index = std::stoi(property.substr(7));
              if (index >= 0 && index < 48)
              {
                iss >> vertex.f_rest[index];
              }
            }
          }
          // Convert colors from unsigned char to the range [0, 1]
          vertex.color.r /= 255.0f;
          vertex.color.g /= 255.0f;
          vertex.color.b /= 255.0f;
          vertex.color.a /= 255.0f;
          vertices.push_back(vertex);
        }
    }

    plyFile.close();

    return vertices;
}