#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <initguid.h>
#include <imgui.h>
#include <wrl/client.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "vector.h"
#include "matrix.h"
#include "d3dx12.h"
#include "DxException.h"

#include "Vertex.h"
#include "PlyReader.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;



// #10 - obsolet with ply parser - MH
//struct Vertex
//{
//    Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, a) {}
//    glm::vec3 pos;
//    glm::vec4 color;
//};

HWND hwnd = NULL;
LPCTSTR WindowName = L"Dreieck";
LPCTSTR WindowTitle = L"Dreieck";
int Width = 800; // of window
int Height = 600;
bool FullScreen = false;
bool Running = true;

std::vector<Vertex> vertices;


bool InitializeWindow(HINSTANCE hInstance,
                      int ShowWnd,
                      bool fullscreen);

struct ConstantBuffer
{
    glm::mat4 projectionMatrix;
};


void mainloop();
LRESULT CALLBACK WndProc(HWND hWnd,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam);

// direct3d stuff
const int frameBufferCount = 3; // number of buffers (2 = double buffering, 3 = tripple buffering)
ComPtr<ID3D12Device> device;
ComPtr<IDXGISwapChain3> swapChain; // swapchain used to switch between render targets
ComPtr<ID3D12CommandQueue> commandQueue;  // container for command lists
ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets
ComPtr<ID3D12Resource> renderTargets[frameBufferCount]; // number of render targets equal to buffer count
ComPtr<ID3D12CommandAllocator> commandAllocator[frameBufferCount]; // enough allocators for each buffer * number of threads
ComPtr<ID3D12GraphicsCommandList> commandList; // add commands, execute to render the frame
ComPtr<ID3D12Fence> fence[frameBufferCount]; // an object that is locked while our command list is being executed by the gpu
ComPtr<ID3D12Resource> constantBufferUploadHeap[frameBufferCount];
ComPtr<ID3D12DescriptorHeap> cbvDescriptorHeap [frameBufferCount];

ConstantBuffer cb;
UINT8* cbColorMultiplierGPUAddress[frameBufferCount];
HANDLE fenceEvent;                                          // a handle to an event when our fence is unlocked by the gpu
UINT64 fenceValue[frameBufferCount];
// this value is incremented each frame. each fence will have its own value
int frameIndex;                                             // current rtv we are on
int rtvDescriptorSize;                                      // size of the rtv descriptor on the device (all front and back buffers will be the same size)

// function declarations
bool InitD3D();                           // initializes direct3d 12
void Update();                            // update the game logic
void UpdatePipeline();                    // update the direct3d pipeline (update command lists)
void Render();                            // execute the command list
void Cleanup();                           // release com ojects and clean up memory
void WaitForPreviousFrame();              // wait until gpu is finished with command list
ID3D12PipelineState *pipelineStateObject; // pso containing a pipeline state
ID3D12RootSignature *rootSignature;       // root signature defines data shaders will access
D3D12_VIEWPORT viewport;                  // area that output from rasterizer will be stretched to.
D3D12_RECT scissorRect;                   // the area to draw in. pixels outside that area will not be drawn onto
ID3D12Resource *vertexBuffer;             // a default buffer in GPU memory that we will load vertex data for our triangle into
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


// Simulierte Funktion, die HRESULT zurückgibt
HRESULT SimulateDirectXFunction() {
    // Hier simulieren wir einen Fehler
    return E_FAIL;  // Simuliere einen Fehlschlag
}

// entry point
int WINAPI WinMain(HINSTANCE hInstance, // Main windows function
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nShowCmd)

{
    AllocConsole();

    
    FILE* fpStdin;
    freopen_s(&fpStdin, "CONIN$", "r", stdin);
    std::cin.clear();

    FILE* fpStdout;
    freopen_s(&fpStdout, "CONOUT$", "w", stdout);
    std::cout.clear();

    FILE* fpStderr;
    freopen_s(&fpStderr, "CONOUT$", "w", stderr);

// #10 start to import PLY file - MH
    std::string plyFilename = "../triangle-data-test.ply";
    //std::string plyFilename = "../bycicle-test.ply";
    vertices = PlyReader::readPlyFile(plyFilename);

    
    // #10 check import success - MH
    std::cout << "Number of imported vertices: " << vertices.size() << std::endl;
    for (size_t i = 0; i < vertices.size(); ++i) {
        const Vertex& vertex = vertices[i];
        std::cout << "Vertex " << i << ": " << std::endl;
        std::cout << "  Position: (" << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << ")" << std::endl;
        std::cout << "  Normale: (" << vertex.normal.x << ", " << vertex.normal.y << ", " << vertex.normal.z << ")" << std::endl;
        std::cout << "  Color: (" << static_cast<int>(vertex.color.r) << ", " << static_cast<int>(vertex.color.g) << ", " << static_cast<int>(vertex.color.b) << ")" << std::endl;
    }
float aspectRatio = Width / Height;
float fov = glm::radians(45.0f);
float zNear = 0.01f;
float zFar  = 100.0f;

glm::mat4 projectionMatrix = glm::perspective(fov, aspectRatio, zNear, zFar);

cb.projectionMatrix = projectionMatrix;

    std::cerr.clear();

    std::wcin.clear();
    std::wcout.clear();
    std::wcerr.clear();
    std::wclog.clear();

    std::ios::sync_with_stdio(true);

    SetConsoleTitle(L"Dreieck Console");
    std::cout << "Hello World" << std::endl;

    //TESTING EXCEPTION WORKING - MH
     try {
        // Testen der DirectX-Funktion mit dem ThrowIfFailed Makro
    ThrowIfFailed(SimulateDirectXFunction());
        }
        catch (const DxException& e) {
    // Fehlermeldung in einer MessageBox anzeigen
    MessageBoxA(NULL, e.what(), "Exception Caught", MB_ICONERROR);
    }
    
    // create the window
    if (!InitializeWindow(hInstance, nShowCmd, FullScreen))
    {
        MessageBox(0, L"Window Initialization - Failed",
                   L"Error", MB_OK);
        return 1;
    }
    // initialize direct3d
    if (!InitD3D())
    {
        MessageBox(0, L"Failed to initialize direct3d 12",
                   L"Error", MB_OK);
        Cleanup();
        return 1;
    }
    // start the main loop
    mainloop();
    // wait for gpu to finish executing the command list before we start releasing everything
    WaitForPreviousFrame();
    // close the fence event
    CloseHandle(fenceEvent);
    // clean up everything
    Cleanup();

    return 0;
}
// create and show the window
bool InitializeWindow(HINSTANCE hInstance,
                      int ShowWnd,
                      bool fullscreen)

{
    if (fullscreen)
    {
        HMONITOR hmon = MonitorFromWindow(hwnd,
                                          MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = {sizeof(mi)};
        GetMonitorInfo(hmon, &mi);

        Width = mi.rcMonitor.right - mi.rcMonitor.left;
        Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WindowName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",
                   L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    hwnd = CreateWindowEx(NULL,
                          WindowName,
                          WindowTitle,
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          Width, Height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    if (!hwnd)
    {
        MessageBox(NULL, L"Error creating window",
                   L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (fullscreen)
    {
        SetWindowLong(hwnd, GWL_STYLE, 0);
    }

    ShowWindow(hwnd, ShowWnd);
    UpdateWindow(hwnd);

    return true;
}

void mainloop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (Running)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // run game code
            Update(); // update the game logic
            Render(); // execute the command queue (rendering the scene is the result of the gpu executing the command lists)
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam)

{
    switch (msg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            if (MessageBox(0, L"Are you sure you want to exit?",
                           L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                Running = false;
                DestroyWindow(hwnd);
            }
        }
        return 0;

    case WM_DESTROY: // x button on top right corner of window was pressed
        Running = false;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd,
                         msg,
                         wParam,
                         lParam);
}

bool InitD3D()
{
    HRESULT hr;

    // -- Enable debug layer -- //
    // MAIGO DID THIS
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        std::cout << "Debug layer ENABLED" << std::endl;
    }

    // -- Create the Device -- //

    IDXGIFactory4 *dxgiFactory;
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIAdapter1 *adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)

    int adapterIndex = 0; // start looking for directx 12  compatible graphics devices starting at index 0
    bool adapterFound = false;

    // find first hardware gpu that supports d3d 12
    while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // we dont want a software device
            continue;
        }

        // -> device that is compatible with direct3d 12
        hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
        if (SUCCEEDED(hr))
        {
            adapterFound = true;
            break;
        }

        adapterIndex++;
    }

    if (!adapterFound)
    {
        return false;
    }

    // Create the device
    hr = D3D12CreateDevice(
        adapter,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device));
    if (FAILED(hr))
    {
        return false;
    }

    // -- Create a direct command queue -- //

    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

    hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)); // create the command queue
    if (FAILED(hr))
    {
        return false;
    }

    // -- Create the Swap Chain (double/tripple buffering) -- //

    DXGI_MODE_DESC backBufferDesc = {};                 // describe display mode
    backBufferDesc.Width = Width;                       // buffer width
    backBufferDesc.Height = Height;                     // buffer height
    backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

    // multisampling -> no multisampling -> value = 1
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = 1; // multisample count

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = frameBufferCount;                // number of buffers
    swapChainDesc.BufferDesc = backBufferDesc;                   // back buffer description
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;    // dxgi will discard the buffer (data) after call present
    swapChainDesc.OutputWindow = hwnd;                           // handle to window
    swapChainDesc.SampleDesc = sampleDesc;                       // multi-sampling description
    swapChainDesc.Windowed = !FullScreen;                        // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

    IDXGISwapChain *tempSwapChain;

    dxgiFactory->CreateSwapChain(
        commandQueue.Get(),   // the queue will be flushed once the swap chain is created
        &swapChainDesc, // give it the swap chain description we created above
        &tempSwapChain  // store the created swap chain in a temp IDXGISwapChain interface
    );

    swapChain = static_cast<IDXGISwapChain3 *>(tempSwapChain);

    frameIndex = swapChain->GetCurrentBackBufferIndex();
    //create constantbufferdescriptionheap for each frame
    //will store constant buffer descriptor
    for (int i = 0; i < frameBufferCount; ++i) {
        // -- Create the Back Buffers (render target views) Descriptor Heap -- //
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 1;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        hr = device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvDescriptorHeap[frameBufferCount]));
        if (FAILED(hr))
        {
            Running = false;
        }
    }
    // describe an rtv descriptor heap and create
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameBufferCount;     // number of descriptors for this heap
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

    // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
    // otherwise set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
    if (FAILED(hr))
    {
        return false;
    }

    // get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors should be stored in it.
    // descriptor sizes may vary from device to device, which is why there is no set size and we must ask the
    // device to give us the size. we will use this size to increment a descriptor handle offset
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // get a handle to the first descriptor in the descriptor heap (handle is like pointer)
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each buffer
    for (int i = 0; i < frameBufferCount; i++)
    {
        // get the n'th buffer in the swap chain and store it in the n'th position of our ID3D12Resource array
        hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        if (FAILED(hr))
        {
            return false;
        }

        //"create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);

        // increment the rtv handle by the rtv descriptor size above
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    // -- Create the Command Allocators -- //

    for (int i = 0; i < frameBufferCount; i++)
    {
        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // -- Create a Command List -- //

    // create the command list with the first allocator
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[frameIndex].Get(), NULL, IID_PPV_ARGS(&commandList));
    if (FAILED(hr))
    {
        return false;
    }

    // -- Create a Fence & Fence Event -- //

    // create the fences
    for (int i = 0; i < frameBufferCount; i++)
    {
        hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
        if (FAILED(hr))
        {
            return false;
        }
        fenceValue[i] = 0; // set the initial fence value to 0
    }

    // create a handle to a fence event
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr)
    {
        return false;
    }

    // create root signature

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ID3DBlob *signature;
    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
    if (FAILED(hr))
    {
        return false;
    }

    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    if (FAILED(hr))
    {
        return false;
    }

    // create vertex and pixel shaders
    // compile vertex shader
    ID3DBlob *vertexShader; // d3d blob for holding vertex shader bytecode
    ID3DBlob *errorBuff;    // a buffer holding the error data if any
    hr = D3DCompileFromFile(L"VertexShader.hlsl",
                            nullptr,
                            nullptr,
                            "main",
                            "vs_5_0",
                            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
                            0,
                            &vertexShader,
                            &errorBuff);
    if (FAILED(hr))
    {
        OutputDebugStringA((char *)errorBuff->GetBufferPointer());
        return false;
    }

    // fill out a shader bytecode structure, which is basically just a pointer to the shader bytecode and the size of the shader bytecode
    D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
    vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
    vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

    // compile pixel shader
    ID3DBlob *pixelShader;
    hr = D3DCompileFromFile(L"PixelShader.hlsl",
                            nullptr,
                            nullptr,
                            "main",
                            "ps_5_0",
                            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
                            0,
                            &pixelShader,
                            &errorBuff);
    if (FAILED(hr))
    {
        OutputDebugStringA((char *)errorBuff->GetBufferPointer());
        return false;
    }

    // fill out shader bytecode structure for pixel shader
    D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
    pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
    pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

    // create input layout
    // The input layout is used by the Input Assembler so that it knows how to read the vertex data bound to it.

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    // fill out an input layout description structure
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

    inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    inputLayoutDesc.pInputElementDescs = inputLayout;

    // create a pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};                        // structure to define a pso
    psoDesc.InputLayout = inputLayoutDesc;                                  // the structure describing our input layout
    psoDesc.pRootSignature = rootSignature;                                 // the root signature that describes the input data this pso needs
    psoDesc.VS = vertexShaderBytecode;                                      // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = pixelShaderBytecode;                                       // same as VS but for pixel shader
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;                     // format of the render target
    psoDesc.SampleDesc = sampleDesc;                                        // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff;                                        // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);       // a default rasterizer state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);                 // a default blent state.
    psoDesc.NumRenderTargets = 1;                                           // only binding one render target

    // create the pso
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
    if (FAILED(hr))
    {
        return false;
    }

    // Create vertex buffer

    //#10 Triangle cords obsolet while running ply parser- MH
    /*Vertex vList[] = {
      {glm::vec3(0.0f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
    {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
    {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)}
    };*/

    int vBufferSize = sizeof(Vertex) * vertices.size();
    std::cout << "Buffersize = " << vBufferSize;

    auto heapPropertiesDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto heapPropertiesUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
   

    // create default heap
    // default heap is memory on the GPU. Only the GPU has access to this memory
    device->CreateCommittedResource(
        &heapPropertiesDefault,         // a default heap
        D3D12_HEAP_FLAG_NONE,           // no flags
        &resourceDesc,                  // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // start this heap in the copy destination state since we will copy data from the upload heap to this heap
        nullptr,                        // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&vertexBuffer));
    vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

    for (int i = 0; i < frameBufferCount; ++i)
    {
        device->CreateCommittedResource(
            &heapPropertiesUpload,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&constantBufferUploadHeap[i]));
        constantBufferUploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");

        //create constant buffer view

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = constantBufferUploadHeap[i]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;
        device->CreateConstantBufferView(&cbvDesc, cbvDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
        
    }

    // create upload heap
    // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
    // upload the vertex buffer using this heap to the default heap
    ID3D12Resource *vBufferUploadHeap;
    device->CreateCommittedResource(
        &heapPropertiesUpload,             // upload heap
        D3D12_HEAP_FLAG_NONE,              // no flags
        &resourceDesc,                     // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&vBufferUploadHeap));
    vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    //vertexData.pData = reinterpret_cast<BYTE *>(vertices); // pointer to our vertex array
    vertexData.pData = vertices.data(); // pointer to our vertex array
    vertexData.RowPitch = vBufferSize;                  // size of all our triangle vertex data
    vertexData.SlicePitch = vBufferSize;                // also the size of our triangle vertex data

    // creating a command with the command list to copy the data from upload heap to default heap
    UpdateSubresources(commandList.Get(), vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

    // transition the vertex buffer data from copy destination state to vertex buffer state
    auto resBarrierVertexBuffer = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    commandList->ResourceBarrier(1, &resBarrierVertexBuffer);

    // execute the command list to upload the initial assets (triangle data)
    commandList->Close();
    ID3D12CommandList *ppCommandLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
    fenceValue[frameIndex]++;
    hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
    if (FAILED(hr))
    {
        Running = false;
    }

    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vBufferSize;

    // Fill out the Viewport
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = Width;
    viewport.Height = Height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // Fill out a scissor rect
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = Width;
    scissorRect.bottom = Height;

    return true;
}
void Update()
{
    // update app logic, such as moving the camera or figuring out what objects are in view
}
void UpdatePipeline()
{
    HRESULT hr;

    // wait for the gpu to finish with the command allocator before we reset it
    WaitForPreviousFrame();

    // only reset an allocator once the gpu is done with it. resetting an allocator frees the memory that the command list was stored in
    hr = commandAllocator[frameIndex]->Reset();
    if (FAILED(hr))
    {
        Running = false;
    }

    // reset the command list
    hr = commandList->Reset(commandAllocator[frameIndex].Get(), pipelineStateObject);
    if (FAILED(hr))
    {
        Running = false;
    }

    // recording commands into the commandList (which all the commands will be stored in the commandAllocator)
    //  transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
    auto resBarrierTransition = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &resBarrierTransition);

    // get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

    // set the render target for the output merger stage (the output of the pipeline)
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear the render target by using the ClearRenderTargetView command
    const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // draw triangle
    commandList->SetGraphicsRootSignature(rootSignature);                     // set the root signature
    commandList->RSSetViewports(1, &viewport);                                // set the viewports
    commandList->RSSetScissorRects(1, &scissorRect);                          // set the scissor rects
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);                 // set the vertex buffer (using the vertex buffer view)
    //#10 Problem
    commandList->DrawInstanced(3, vertices.size() / 3, 0, 0);                 // finally draw 3 vertices (draw the triangle)

    // transition the "frameIndex" render target from the render target state to the present state
    auto resBarrierTransPresent = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &resBarrierTransPresent);

    hr = commandList->Close();
    if (FAILED(hr))
    {
        Running = false;
    }
}
void Render()
{
    HRESULT hr;

    UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

    // create an array of command lists (only one command list here)
    ID3D12CommandList *ppCommandLists[] = {commandList.Get()};

    // execute the array of command lists
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // this command goes in at the end of our command queue. we will know when our command queue
    // has finished because the fence value will be set to "fenceValue" from the GPU since the command
    // queue is being executed on the GPU
    hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
    if (FAILED(hr))
    {
        Running = false;
    }

    // present the current backbuffer
    hr = swapChain->Present(0, 0);
    if (FAILED(hr))
    {
        Running = false;
    }
}

void Cleanup()
{
    // wait for the gpu to finish all frames
    for (int i = 0; i < frameBufferCount; ++i)
    {
        frameIndex = i;
        WaitForPreviousFrame();
    }

    // get swapchain out of full screen before exiting
    BOOL fs = false;
    if (swapChain->GetFullscreenState(&fs, NULL))
        swapChain->SetFullscreenState(false, NULL);

    device.Reset();
    swapChain.Reset();
    commandQueue.Reset();
    rtvDescriptorHeap.Reset();
    commandList.Reset();

    for (int i = 0; i < frameBufferCount; ++i)
    {
        renderTargets[i].Reset();
        commandAllocator[i].Reset();
        fence[i].Reset();
    };

    pipelineStateObject->Release();
    rootSignature->Release();
    vertexBuffer->Release();
}

void WaitForPreviousFrame()
{
    HRESULT hr;

    // swap the current rtv buffer index so we draw on the correct buffer
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
    // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
    if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
    {
        // fence create an event which is signaled once the fence's current value is "fenceValue"
        hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
        if (FAILED(hr))
        {
            Running = false;
        }

        // wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
        // has reached "fenceValue", we know the command queue has finished executing
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    // increment fenceValue for next frame
    fenceValue[frameIndex]++;
}

