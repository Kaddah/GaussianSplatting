#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <iostream>

// Enum for shader type
enum class ShaderType {
	Vertex,
	Pixel,
	Geometry,
	Compute
};

bool CompileShader(const std::wstring& shaderFilePath, 
	ShaderType shaderType,
	ID3DBlob** shaderBlob,
	ID3DBlob** errorBlob);