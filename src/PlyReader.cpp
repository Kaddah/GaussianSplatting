#include "PlyReader.h"
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>
#include <windows.h>

std::vector<Vertex> PlyReader::readPlyFile(const std::string& filename)
{
  auto start = std::chrono::high_resolution_clock::now();

  HANDLE fileHandle = CreateFileA(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    std::cerr << "Konnte die PLY-Datei nicht öffnen." << std::endl;
    return std::vector<Vertex>();
  }

  LARGE_INTEGER fileSize;
  if (!GetFileSizeEx(fileHandle, &fileSize))
  {
    std::cerr << "Konnte die Datei-Informationen nicht abrufen." << std::endl;
    CloseHandle(fileHandle);
    return std::vector<Vertex>();
  }

  HANDLE mappingHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
  if (mappingHandle == NULL)
  {
    std::cerr << "Konnte die Datei nicht speicherabbilden." << std::endl;
    CloseHandle(fileHandle);
    return std::vector<Vertex>();
  }

  char* fileData = static_cast<char*>(MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, fileSize.QuadPart));
  if (fileData == NULL)
  {
    std::cerr << "Konnte die Datei nicht speicherabbilden." << std::endl;
    CloseHandle(mappingHandle);
    CloseHandle(fileHandle);
    return std::vector<Vertex>();
  }

  CloseHandle(mappingHandle);
  CloseHandle(fileHandle);

  std::string              format;
  std::vector<std::string> properties;
  bool                     isBinary    = false;
  int                      vertexCount = 0;

  // Read header
  char* cursor = fileData;
  char* end    = fileData + fileSize.QuadPart;
  while (cursor < end)
  {
    char*       lineEnd = std::find(cursor, end, '\n');
    std::string line(cursor, lineEnd);
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
    else if (line.substr(0, 15) == "element vertex ")
    {
      vertexCount = std::stoi(line.substr(15));
    }
    else if (line.substr(0, 8) == "property")
    {
      std::istringstream iss(line);
      std::string        propertyType, propertyName;
      iss >> propertyType >> propertyType >> propertyName;
      properties.push_back(propertyName);
    }
    else if (line == "end_header")
    {
      cursor = lineEnd + 1;
      break;
    }
    cursor = lineEnd + 1;
  }

  std::vector<Vertex> vertices;
  vertices.reserve(vertexCount);

  using PropertyReaderBinary = std::function<void(char*&, Vertex&)>;
  std::unordered_map<std::string, PropertyReaderBinary> propertyReadersBinary;

  propertyReadersBinary["x"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.pos.x, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["y"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.pos.y, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["z"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.pos.z, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["nx"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.normal.x, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["ny"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.normal.y, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["nz"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.normal.z, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["red"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.color.r, data, sizeof(unsigned char));
    data += sizeof(unsigned char);
  };
  propertyReadersBinary["green"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.color.g, data, sizeof(unsigned char));
    data += sizeof(unsigned char);
  };
  propertyReadersBinary["blue"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.color.b, data, sizeof(unsigned char));
    data += sizeof(unsigned char);
  };
  propertyReadersBinary["alpha"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.color.a, data, sizeof(unsigned char));
    data += sizeof(unsigned char);
  };
  propertyReadersBinary["opacity"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.opacity, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["scale_0"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.scale.x, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["scale_1"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.scale.y, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["scale_2"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.scale.z, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["rot_0"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.rotation.x, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["rot_1"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.rotation.y, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["rot_2"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.rotation.z, data, sizeof(float));
    data += sizeof(float);
  };
  propertyReadersBinary["rot_3"] = [](char*& data, Vertex& vertex)
  {
    std::memcpy(&vertex.rotation.w, data, sizeof(float));
    data += sizeof(float);
  };
  for (int i = 0; i < 3; ++i)
  {
    propertyReadersBinary["f_dc_" + std::to_string(i)] = [i](char*& data, Vertex& vertex)
    {
      std::memcpy(&vertex.f_rest[i], data, sizeof(float));
      data += sizeof(float);
    };
  }

  for (int i = 0; i < 45; ++i)
  {
    propertyReadersBinary["f_rest_" + std::to_string(i)] = [i](char*& data, Vertex& vertex)
    {
      std::memcpy(&vertex.f_rest[3+i], data, sizeof(float));
      data += sizeof(float);
    };
  }

  using PropertyReaderAscii = std::function<void(std::istringstream&, Vertex&)>;
  std::unordered_map<std::string, PropertyReaderAscii> propertyReadersAscii;

  propertyReadersAscii["x"]       = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.pos.x; };
  propertyReadersAscii["y"]       = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.pos.y; };
  propertyReadersAscii["z"]       = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.pos.z; };
  propertyReadersAscii["nx"]      = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.normal.x; };
  propertyReadersAscii["ny"]      = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.normal.y; };
  propertyReadersAscii["nz"]      = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.normal.z; };
  propertyReadersAscii["red"]     = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.color.r; };
  propertyReadersAscii["green"]   = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.color.g; };
  propertyReadersAscii["blue"]    = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.color.b; };
  propertyReadersAscii["alpha"]   = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.color.a; };
  propertyReadersAscii["opacity"] = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.opacity; };
  propertyReadersAscii["scale_0"] = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.scale.x; };
  propertyReadersAscii["scale_1"] = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.scale.y; };
  propertyReadersAscii["scale_2"] = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.scale.z; };
  propertyReadersAscii["rot_0"]   = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.rotation.x; };
  propertyReadersAscii["rot_1"]   = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.rotation.y; };
  propertyReadersAscii["rot_2"]   = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.rotation.z; };
  propertyReadersAscii["rot_3"]   = [](std::istringstream& iss, Vertex& vertex) { iss >> vertex.rotation.w; };

  for (int i = 0; i < 48; ++i)
  {
    propertyReadersAscii["f_rest_" + std::to_string(i)] = [i](std::istringstream& iss, Vertex& vertex)
    { iss >> vertex.f_rest[i]; };
  }

  if (isBinary)
  {
    // Read binary data in one go
    while (cursor < end)
    {
      Vertex vertex = {};
      for (const auto& property : properties)
      {
        if (propertyReadersBinary.find(property) != propertyReadersBinary.end())
        {
          propertyReadersBinary[property](cursor, vertex);
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
    std::vector<std::string> lines;
    lines.reserve(vertexCount);

    while (cursor < end)
    {
      char* lineEnd = std::find(cursor, end, '\n');
      lines.emplace_back(cursor, lineEnd);
      cursor = lineEnd + 1;
    }

    std::vector<std::future<void>> futures;
    const size_t                   numThreads     = std::thread::hardware_concurrency();
    const size_t                   linesPerThread = lines.size() / numThreads;

    for (size_t t = 0; t < numThreads; ++t)
    {
      futures.push_back(std::async(std::launch::async,
                                   [&, t]()
                                   {
                                     size_t startIdx = t * linesPerThread;
                                     size_t endIdx   = (t == numThreads - 1) ? lines.size() : (t + 1) * linesPerThread;
                                     for (size_t i = startIdx; i < endIdx; ++i)
                                     {
                                       std::istringstream iss(lines[i]);
                                       Vertex             vertex = {};
                                       for (const auto& property : properties)
                                       {
                                         if (propertyReadersAscii.find(property) != propertyReadersAscii.end())
                                         {
                                           propertyReadersAscii[property](iss, vertex);
                                         }
                                       }
                    
                                       vertex.color.r /= 255.0f;
                                       vertex.color.g /= 255.0f;
                                       vertex.color.b /= 255.0f;
                                       vertex.color.a /= 255.0f;
                                       vertices.push_back(vertex);
                                     }
                                   }));
    }

    for (auto& future : futures)
    {
      future.get();
    }
  }

  UnmapViewOfFile(fileData);

  auto                          endTime  = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = endTime - start;
  std::cout << "Time taken to read PLY file: " << duration.count() << " seconds" << std::endl;



  return vertices;
}

