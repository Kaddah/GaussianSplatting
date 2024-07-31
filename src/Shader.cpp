#include "Shader.h"

bool CompileShader(const std::wstring& shaderFilePath, ShaderType shaderType, ID3DBlob** shaderBlob,
                   ID3DBlob** errorBlob)
{
  // Determine the target shader model on the shader type
  const char* target = nullptr;
  switch (shaderType)
  {
    case ShaderType::Vertex:
      target = "vs_5_0";
      break;
    case ShaderType::Pixel:
      target = "ps_5_0";
      break;
    case ShaderType::Geometry:
      target = "gs_5_0";
      break;
    case ShaderType::Compute:
      target = "cs_5_0";
      break;
    default:
      std::cerr << "Unknown shader type." << std::endl;
      return false;
  }

  HRESULT hr =
      D3DCompileFromFile(shaderFilePath.c_str(), // Path to shader file
                         nullptr,                // Optional macros
                         nullptr,                // Optional include handler
                         "main", target, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, shaderBlob, errorBlob);

  if (FAILED(hr))
  {
    if (*errorBlob)
    {
      // std::cerr << "Shader compilation failed:\n" << (char*)(*errorBlob)->GetBufferPointer() << std::endl;
      std::cerr << "Shader compilation failed:\n"
                << static_cast<const char*>((*errorBlob)->GetBufferPointer()) << std::endl;
    }
    else
    {
      std::cerr << "Shader compilation failed with HRESULT: " << std::hex << hr << std::endl;
    }
    return false;
  }
  else
  {
    std::cout << "Shader compiled successfully." << std::endl;
    return true;
  }
}