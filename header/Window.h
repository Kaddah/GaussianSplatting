#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <glm/glm.hpp>
#include <D3Dcompiler.h>

using Microsoft::WRL::ComPtr;

constexpr int frameBufferCount = 3; // number of buffers (2 = double buffering, 3 = tripple buffering)

struct Vertex
{
	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, z) {}
	glm::vec3 pos;
	glm::vec4 color;
};

class Window {
public:
	Window(LPCTSTR WindowName,
		int width, // of window
		int height,
		bool fullScreen, 
		HINSTANCE hInstance, 
		int nShowCmd);

	virtual void draw() = 0;
	void Stop();
	void WaitForPreviousFrame();
	void Render();
	~Window();

protected:
	int _width;
	int _height;
	bool _running;
	bool _fullScreen;
	HWND _hwnd;

	ComPtr<ID3D12Device> device;
	ComPtr<IDXGISwapChain3> swapChain; // swapchain used to switch between render targets
	ComPtr<ID3D12CommandQueue> commandQueue;  // container for command lists
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets
	ComPtr<ID3D12Resource> renderTargets[frameBufferCount]; // number of render targets equal to buffer count
	ComPtr<ID3D12CommandAllocator> commandAllocator[frameBufferCount]; // enough allocators for each buffer * number of threads
	ComPtr<ID3D12GraphicsCommandList> commandList; // add commands, execute to render the frame
	ComPtr<ID3D12Fence> fence[frameBufferCount];    // an object that is locked while our command list is being executed by the gpu

	HANDLE fenceEvent;                                          // a handle to an event when our fence is unlocked by the gpu
	UINT64 fenceValue[frameBufferCount];                        // this value is incremented each frame. each fence will have its own value
	int frameIndex;                                             // current rtv we are on
	int rtvDescriptorSize;                                      // size of the rtv descriptor on the device (all front and back buffers will be the same size)
	ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state
	ID3D12RootSignature* rootSignature;       // root signature defines data shaders will access
	D3D12_VIEWPORT viewport;                  // area that output from rasterizer will be stretched to.
	D3D12_RECT scissorRect;                   // the area to draw in. pixels outside that area will not be drawn onto
	ID3D12Resource* vertexBuffer;             // a default buffer in GPU memory that we will load vertex data for our triangle into
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


	bool InitializeWindow(HINSTANCE hInstance,
		int ShowWnd,
		bool fullscreen, LPCWSTR windowName);

	bool InitD3D();
	void UpdatePipeline();

};