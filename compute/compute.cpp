#include "../header/d3dx12.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <wrl/client.h>

using namespace Microsoft::WRL;
using namespace std;

// Definition der Datenstruktur float2
struct float2
{
  float x, y;
};

// Hilfsfunktion zum Kompilieren des Shaders
ComPtr<ID3DBlob> CompileShader(const wchar_t* filename, const char* entrypoint, const char* target)
{
  UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
  compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ComPtr<ID3DBlob> byteCode = nullptr;
  ComPtr<ID3DBlob> errors;
  HRESULT          hr = D3DCompileFromFile(filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint, target,
                                           compileFlags, 0, &byteCode, &errors);

  if (errors != nullptr)
  {
    // Ausführliche Fehlermeldung im Debug Output
    std::stringstream errorMessage;
    errorMessage << "Shader-Kompilierung fehlgeschlagen:\n";
    errorMessage << static_cast<char*>(errors->GetBufferPointer()) << "\n";
    OutputDebugStringA(errorMessage.str().c_str());
  }

  if (FAILED(hr))
  {
    throw std::runtime_error("Fehler beim Kompilieren des Shaders");
  }
  return byteCode;
}

int main()
{
  try
  {

    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
      debugController->EnableDebugLayer();
      std::cout << "Debug layer ENABLED" << std::endl;
    }

    // Initialisiere DirectX 12
    ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT               hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create DXGI factory");
    }

    ComPtr<ID3D12Device> device;
    hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create D3D12 device");
    }

    // Erstelle Befehlswarteschlange (Command Queue)
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ComPtr<ID3D12CommandQueue> commandQueue;
    hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create command queue");
    }

    // Erstelle Befehlszuweiser (Command Allocator) und Befehlsliste (Command List)
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create command allocator");
    }

    ComPtr<ID3D12GraphicsCommandList> commandList;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
                                   IID_PPV_ARGS(&commandList));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create command list");
    }

    // Erstelle Compute Shader
    ComPtr<ID3DBlob> computeShader = CompileShader(L"compute/compute.hlsl", "main", "cs_5_0");

    if (!computeShader)
    {
      throw std::runtime_error("Compute shader compilation returned a null blob");
    }

    // Erstelle Root-Signatur
    D3D12_ROOT_PARAMETER rootParameters[1];
    // Konfiguration der Descriptor-Table für UAV
    // (Annahme: Nur eine Descriptor-Range für den UAV)
    D3D12_DESCRIPTOR_RANGE descriptorTableRanges[1];
    descriptorTableRanges[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorTableRanges[0].NumDescriptors                    = 1;
    descriptorTableRanges[0].BaseShaderRegister                = 0;
    descriptorTableRanges[0].RegisterSpace                     = 0;
    descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameters[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges);
    rootParameters[0].DescriptorTable.pDescriptorRanges   = descriptorTableRanges;
    rootParameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

    // Konfiguration der Root-Signatur
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters             = _countof(rootParameters);
    rootSignatureDesc.pParameters               = rootParameters;
    rootSignatureDesc.NumStaticSamplers         = 0;
    rootSignatureDesc.pStaticSamplers           = nullptr;
    rootSignatureDesc.Flags                     = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    // Serialisieren der Root-Signatur
    ComPtr<ID3DBlob> serializedRootSig;
    ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    if (FAILED(hr))
    {
      if (errorBlob != nullptr)
      {
        OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
      }
      throw std::runtime_error("Failed to serialize root signature");
    }

    // Erstellen der Root-Signatur
    ComPtr<ID3D12RootSignature> rootSignature;
    hr = device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(),
                                     IID_PPV_ARGS(&rootSignature));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create root signature");
    }

    // Erstellen des Compute Pipeline State Objects (PSO)
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature                    = rootSignature.Get();
    psoDesc.CS                                = {computeShader->GetBufferPointer(), computeShader->GetBufferSize()};

    ComPtr<ID3D12PipelineState> pipelineState;
    hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
    if (FAILED(hr))
    {
      if (hr == E_INVALIDARG)
      {
        throw std::runtime_error("Invalid arguments when creating compute pipeline state");
      }
      else if (hr == E_OUTOFMEMORY)
      {
        throw std::runtime_error("Out of memory when creating compute pipeline state");
      }
      else
      {
        throw std::runtime_error("Failed to create compute pipeline state");
      }
    }

    // Erstelle Eingabe- und Ausgabepuffer
    vector<float2> input = {{1.0f, 1.0f}};
    vector<float2> output(1);

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width               = sizeof(float2) * input.size(); // Größe des Puffers entsprechend der Daten
    bufferDesc.Height              = 1;                             // Bei Buffern sollte die Höhe 1 sein
    bufferDesc.DepthOrArraySize    = 1;                   // Bei Buffern sollte die Tiefe oder Arraygröße 1 sein
    bufferDesc.MipLevels           = 1;                   // Anzahl der Mip-Levels, normalerweise 1 für Buffers
    bufferDesc.Format              = DXGI_FORMAT_UNKNOWN; // DXGI_FORMAT_UNKNOWN für Buffers
    bufferDesc.SampleDesc.Count    = 1;                   // Anzahl der Samples, normalerweise 1 für Buffers
    bufferDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // Layout für Buffers

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    // Erstellen des Eingabepuffers
    ComPtr<ID3D12Resource> inputBuffer;
    hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // Heap-Eigenschaften
                                         D3D12_HEAP_FLAG_NONE,                              // Flags für die Erstellung
                                         &bufferDesc,                       // Beschreibung des Ressourcenobjekts
                                         D3D12_RESOURCE_STATE_GENERIC_READ, // Anfangszustand der Ressource
                                         nullptr,                   // Übergabe für vorhandenes Ressourcenobjekt
                                         IID_PPV_ARGS(&inputBuffer) // IID des zurückgegebenen Schnittstellenpunkts
    );
    if (FAILED(hr))
    {
      std::stringstream errorMessage;
      errorMessage << "Failed to create input buffer. HRESULT = 0x" << std::hex << hr;
      throw std::runtime_error(errorMessage.str());
    }
    std::cout << "Input buffer created successfully" << std::endl;

    bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    // Erstellen des Ausgabepuffers
    ComPtr<ID3D12Resource> outputBuffer;
    hr = device->CreateCommittedResource(&heapProps,                            // Heap-Eigenschaften
                                         D3D12_HEAP_FLAG_NONE,                  // Flags für die Erstellung
                                         &bufferDesc,                           // Beschreibung des Ressourcenobjekts
                                         D3D12_RESOURCE_STATE_UNORDERED_ACCESS, // Anfangszustand der Ressource
                                         nullptr,                      // Übergabe für vorhandenes Ressourcenobjekt
                                         IID_PPV_ARGS(&outputBuffer)); // IID des zurückgegebenen Schnittstellenpunkts
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create output buffer");
    }

    // Erstelle UAV für den Ausgabepuffer
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format                           = DXGI_FORMAT_R32_FLOAT;
    uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements               = static_cast<UINT>(output.size());

    ComPtr<ID3D12DescriptorHeap> uavHeap;
    D3D12_DESCRIPTOR_HEAP_DESC   uavHeapDesc = {};
    uavHeapDesc.NumDescriptors               = 1; // Anzahl der Descriptors im Heap
    uavHeapDesc.Type                         = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    uavHeapDesc.Flags                        = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr                                       = device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&uavHeap));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create UAV descriptor heap");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE uavHandle    = uavHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE uavGpuHandle = uavHeap->GetGPUDescriptorHandleForHeapStart();

    device->CreateUnorderedAccessView(outputBuffer.Get(), nullptr, &uavDesc,
                                      uavHeap->GetCPUDescriptorHandleForHeapStart());

    // Mapping des Eingabepuffers
    std::cout << "Mapping input buffer..." << std::endl;
    void*       pMappedData = nullptr;
    D3D12_RANGE readRange   = {}; // Das gesamte Ressourcenpaket mappen

    hr = inputBuffer->Map(0, &readRange, &pMappedData);
    if (FAILED(hr))
    {
      std::stringstream errorMessage;
      errorMessage << "Failed to map input buffer. HRESULT = 0x" << std::hex << hr;
      throw std::runtime_error(errorMessage.str());
    }

    if (pMappedData == nullptr)
    {
      throw std::runtime_error("Mapped data pointer is nullptr.");
    }

    memcpy(pMappedData, input.data(), sizeof(float2) * input.size());
    inputBuffer->Unmap(0, nullptr);

    // Aufzeichne Befehle
    commandList->SetPipelineState(pipelineState.Get());
    commandList->SetComputeRootSignature(rootSignature.Get());
    commandList->SetDescriptorHeaps(1, uavHeap.GetAddressOf());
    commandList->SetComputeRootDescriptorTable(0, uavHeap->GetGPUDescriptorHandleForHeapStart());
    commandList->Dispatch(1, 1, 1);

    // Führe die Befehlsliste aus
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Warte auf Beendigung durch die GPU
    ComPtr<ID3D12Fence> fence;
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create fence");
    }

    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr)
    {
      throw std::runtime_error("Failed to create fence event");
    }

    UINT64 fenceValue = 1;
    commandQueue->Signal(fence.Get(), fenceValue);

    hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to set fence event on completion");
    }

    WaitForSingleObject(fenceEvent, INFINITE);
    CloseHandle(fenceEvent);

    // Lese Daten zurück
    hr = outputBuffer->Map(0, nullptr, &pMappedData);
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to map output buffer");
    }
    memcpy(output.data(), pMappedData, sizeof(float2) * output.size());
    outputBuffer->Unmap(0, nullptr);

    // Gebe die Ergebnisse aus
    for (const auto& point : output)
    {
      cout << "Punkt: (" << point.x << ", " << point.y << ")" << endl;
    }

    // Bereinige Ressourcen
    // CloseHandle(eventHandle);

    return 0;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Fehler aufgetreten: " << e.what() << std::endl;
    return 1;
  }
}
