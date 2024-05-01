#include <stdexcept> 
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <iostream>

#include "d3dx12.h"
#include "Window.h"
#include "WipImgui.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

Window::Window(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd): _width(width), _height(height), _hwnd(NULL), _running(true), _fullScreen(fullScreen) //initializer list
{
    if (!InitializeWindow(hInstance, nShowCmd, fullScreen, WindowName))
    {
        throw std::runtime_error("Failed to initialize window");
    }
    if (!InitD3D())
    {
        MessageBox(0, L"Failed to initialize direct3d 12",
            L"Error", MB_OK);
    }
     
}

void Window::Stop()
{
    _running = false;
}

Window::~Window()
{
    //Cleanup
        // wait for the gpu to finish all frames
        WaitForPreviousFrame();
        // get swapchain out of full screen before exiting
        BOOL fs = false;
        if (swapChain->GetFullscreenState(&fs, NULL))
            swapChain->SetFullscreenState(false, NULL);
        killImgui();
        CloseHandle(fenceEvent);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Window::getRTVHandle()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
    return rtvHandle;
}

LRESULT CALLBACK WndProc(HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)

{
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    Window* window = nullptr;
    if (msg == WM_NCCREATE) {
        // Set the pointer to window instance
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }
    else {
        // Retrieve the pointer to window instance
        window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }


    switch (msg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            if (MessageBox(0, L"Are you sure you want to exit?",
                L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                window->Stop();
                DestroyWindow(hwnd);
            }
        }
        return 0;

    case WM_DESTROY: // x button on top right corner of window was pressed
        window->Stop();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd,
        msg,
        wParam,
        lParam);
}

bool Window::InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen, LPCWSTR windowName)
{
    if (fullscreen)
    {
        HMONITOR hmon = MonitorFromWindow(NULL,
            MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hmon, &mi);

        _width = mi.rcMonitor.right - mi.rcMonitor.left;
        _height = mi.rcMonitor.bottom - mi.rcMonitor.top;
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
    wc.lpszClassName = windowName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

     _hwnd = CreateWindowEx(NULL,
        windowName,
        windowName, //windowTitle
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        _width, _height,
        NULL,
        NULL,
        hInstance,
        this);

    if (!_hwnd)
    {
        MessageBox(NULL, L"Error creating window",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (fullscreen)
    {
        SetWindowLong(_hwnd, GWL_STYLE, 0);
    }

    ShowWindow(_hwnd, ShowWnd);
    UpdateWindow(_hwnd);

    return true;
}


bool Window::InitD3D()
{
    HRESULT hr;

    //Enable Debug Layer
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        std::cout << "Debug layer ENABLED" << std::endl;
    }

    //Create Device
    IDXGIFactory4* dxgiFactory;
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIAdapter1* adapter; //adapter = graphics card

    int adapterIndex = 0; 
    bool adapterFound = false;

    // find first hardware gpu that supports d3d 12
    while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
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

    // Create device
    hr = D3D12CreateDevice(
        adapter,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device));
    if (FAILED(hr))
    {
        return false;
    }

    //Create a direct command queue
    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 

    hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)); // create the command queue
    if (FAILED(hr))
    {
        return false;
    }

    //Create the Swap Chain (double/tripple buffering)
    DXGI_MODE_DESC backBufferDesc = {};                 
    backBufferDesc.Width = _width;                       
    backBufferDesc.Height = _height;                     
    backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 

    // multisampling -> no multisampling -> value = 1
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = 1; // multisample count

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = frameBufferCount;                
    swapChainDesc.BufferDesc = backBufferDesc;                   
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //pipeline will render to this swap chain
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;    // dxgi will discard the buffer (data) after call present
    swapChainDesc.OutputWindow = _hwnd;              
    swapChainDesc.SampleDesc = sampleDesc;                       
    swapChainDesc.Windowed = !_fullScreen;                        

    IDXGISwapChain* tempSwapChain;

    dxgiFactory->CreateSwapChain(
        commandQueue.Get(),   
        &swapChainDesc, 
        &tempSwapChain 
    );

    swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

    frameIndex = swapChain->GetCurrentBackBufferIndex();

    //Create the Back Buffers (render target views) Descriptor Heap 
    // describe an rtv descriptor heap and create
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameBufferCount;     
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; 

    // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
    // otherwise set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
    if (FAILED(hr))
    {
        return false;
    }

    // get the size of a descriptor in this heap
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

    //Create the Command Allocators
    for (int i = 0; i < frameBufferCount; i++)
    {
        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
        if (FAILED(hr))
        {
            return false;
        }
    }
 
    // create the command list with the first allocator
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[frameIndex].Get(), NULL, IID_PPV_ARGS(&commandList));
    if (FAILED(hr))
    {
        return false;
    }

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

    ID3DBlob* signature;
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
    ID3DBlob* vertexShader; // d3d blob for holding vertex shader bytecode
    ID3DBlob* errorBuff;    // a buffer holding the error data if any
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
        OutputDebugStringA((char*)errorBuff->GetBufferPointer());
        return false;
    }

    // fill out a shader bytecode structure (which is basically just a pointer to the shader bytecode and the size of the shader bytecode)
    D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
    vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
    vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

    // compile pixel shader
    ID3DBlob* pixelShader;
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
        OutputDebugStringA((char*)errorBuff->GetBufferPointer());
        return false;
    }

    // fill out shader bytecode structure for pixel shader
    D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
    pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
    pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

    // create input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

    // fill out an input layout description structure
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

    inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    inputLayoutDesc.pInputElementDescs = inputLayout;

    // create a pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};                        
    psoDesc.InputLayout = inputLayoutDesc;                                  
    psoDesc.pRootSignature = rootSignature;                                 
    psoDesc.VS = vertexShaderBytecode;                                      
    psoDesc.PS = pixelShaderBytecode;                                       
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;                     // format of the render target
    psoDesc.SampleDesc = sampleDesc;                                        // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff;                                        // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);       
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);                 
    psoDesc.NumRenderTargets = 1;                                           

    // create the pso
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
    if (FAILED(hr))
    {
        return false;
    }

    // Create vertex buffer
    Vertex vList[] = {
        {0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f},
        {0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f},
    };

    int vBufferSize = sizeof(vList);

    auto heapPropertiesDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto heapPropertiesUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);

    // create default heap
    device->CreateCommittedResource(
        &heapPropertiesDefault,         
        D3D12_HEAP_FLAG_NONE,           
        &resourceDesc,                  
        D3D12_RESOURCE_STATE_COPY_DEST, 
        nullptr,                        
        IID_PPV_ARGS(&vertexBuffer));
    vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

    // create upload heap
    ID3D12Resource* vBufferUploadHeap;
    device->CreateCommittedResource(
        &heapPropertiesUpload,             
        D3D12_HEAP_FLAG_NONE,              
        &resourceDesc,                     
        D3D12_RESOURCE_STATE_GENERIC_READ, 
        nullptr,
        IID_PPV_ARGS(&vBufferUploadHeap));
    vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
    vertexData.RowPitch = vBufferSize;                  // size of all our triangle vertex data
    vertexData.SlicePitch = vBufferSize;                // also the size of our triangle vertex data

    // creating a command with the command list to copy the data from upload heap to default heap
    UpdateSubresources(commandList.Get(), vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

    // transition the vertex buffer data from copy destination state to vertex buffer state
    auto resBarrierVertexBuffer = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    commandList->ResourceBarrier(1, &resBarrierVertexBuffer);

    // execute the command list to upload the initial assets (triangle data)
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
    fenceValue[frameIndex]++;
    hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
    if (FAILED(hr))
    {
        _running = false;
    }

    if (SUCCEEDED(hr)) {
        initImgui(device.Get(), frameBufferCount, _hwnd);
    }
    else {
        return false;
    }



    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vBufferSize;

    // Fill out the Viewport
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = _width;
    viewport.Height = _height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // Fill out a scissor rect
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = _width;
    scissorRect.bottom = _height;

    return true;
}

void Window::Render()
{
    HRESULT hr;

    UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

    // create an array of command lists (only one command list here)
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };

    // execute the array of command lists
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
    if (FAILED(hr))
    {
        _running = false;
    }

    // present the current backbuffer
    hr = swapChain->Present(0, 0);
    if (FAILED(hr))
    {
        _running = false;
    }
    endMainImgui(commandList.Get());
}

void Window::mainloop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (_running)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                _running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!_running) {
            break;
        }

        startMainImgui();

        Render(); // execute the command queue


         
    }
}

void Window::UpdatePipeline()
{
    HRESULT hr;

    // wait for the gpu to finish with the command allocator before we reset it
    WaitForPreviousFrame();

    // only reset an allocator once the gpu is done with it. resetting an allocator frees the memory that the command list was stored in
    hr = commandAllocator[frameIndex]->Reset();
    if (FAILED(hr))
    {
        _running = false;
        return;
    }

    // reset the command list
    hr = commandList->Reset(commandAllocator[frameIndex].Get(), pipelineStateObject);
    if (FAILED(hr))
    {
        _running = false;
        return;
    }

    // recording commands into the commandList (which all the commands will be stored in the commandAllocator)
    //  transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
    auto resBarrierTransition = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &resBarrierTransition);

    draw();

    // transition the "frameIndex" render target from the render target state to the present state
    auto resBarrierTransPresent = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &resBarrierTransPresent);

    hr = commandList->Close();
    if (FAILED(hr))
    {
        _running = false;
    }
}

void Window::WaitForPreviousFrame()
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
            _running = false;
        }

        // wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
        // has reached "fenceValue", we know the command queue has finished executing
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    // increment fenceValue for next frame
    fenceValue[frameIndex]++;
}

