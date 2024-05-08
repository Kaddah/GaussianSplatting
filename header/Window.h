#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <D3Dcompiler.h>
#include <wrl/client.h>
#include <d3dx12.h>
#include <Vertex.h>



using Microsoft::WRL::ComPtr;

constexpr int frameBufferCount = 3; // number of buffers (2 = double buffering, 3 = tripple buffering)

//struct Vertex
//{
//	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, z) {}
//	glm::vec3 pos;
//	glm::vec4 color;
//};
struct ConstantBuffer {
	glm::mat4 wvpMat;
	

};

int ConstantBufferAlignedSize = (sizeof(ConstantBuffer) + 255) & ~255;
ConstantBuffer cbObj;

class Maths {
private:
	glm::mat4 cameraProjMat; // this will store our projection matrix
	glm::mat4 cameraViewMat; // this will store our view matrix

	glm::vec4 cameraPosition; // this is our camera's position vector
	glm::vec4 cameraTarget; // a vector describing the point in space our camera is looking at
	glm::vec4 cameraUp; // the world's up vector

	glm::mat4 objectWorldMat; // object's world matrix (transformation matrix)
	glm::vec4 objectPosition; // object's position in space

public:
	Maths() {
		cameraPosition = glm::vec4(0.0f, 2.0f, -4.0f, 1.0f); // Set w component to 1 for position
		cameraTarget = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Set w component to 1 for position
		cameraUp = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // Set w component to 0 for direction
		cameraViewMat = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(cameraTarget), glm::vec3(cameraUp));

		// Set up projection matrix
		float aspectRatio = 800.0f / 600.0f; // example aspect ratio
		float fov = glm::radians(45.0f); // example field of view
		float nearPlane = 0.1f; // example near plane
		float farPlane = 1000.0f; // example far plane
		cameraProjMat = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

		// Set up object parameters
		objectPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Set w component to 1 for position
		objectWorldMat = glm::translate(glm::mat4(1.0f), glm::vec3(objectPosition));

		// Update camera position to rotate around object
		float radius = 4.0f; // example radius
		float angle = glm::radians(45.0f); // example angle
		float cameraX = objectPosition.x + radius * glm::cos(angle);
		float cameraZ = objectPosition.z + radius * glm::sin(angle);
		cameraPosition = glm::vec4(cameraX, 2.0f, cameraZ, 1.0f); // Set w component to 1 for position

		// Update view matrix based on new camera position
		cameraViewMat = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(cameraTarget), glm::vec3(cameraUp));

		// Apply transformations to object
		// For example:
		// Rotate the object over time
		float deltaTime = 0.016f; // example delta time
		float rotationSpeed = glm::radians(30.0f); // example rotation speed (30 degrees per second)
		objectWorldMat = glm::rotate(objectWorldMat, rotationSpeed * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f)); // rotating around Y-axis

		// Combine view and projection matrix
		glm::mat4 MVP = cameraProjMat * cameraViewMat * objectWorldMat; // Model-View-Projection matrix

	}
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
	//virtual std::vector<Vertex> prepareTriangle()=0;
	virtual std::vector<Vertex>  prepareTriangle() = 0;
	void Stop();
	void WaitForPreviousFrame();
	void Render();
	void mainloop();
	
	void UpdateVertexBuffer(const std::vector<Vertex>& vertices);
	bool InitializeVertexBuffer(const std::vector<Vertex>& vertices);
	~Window();

	CD3DX12_CPU_DESCRIPTOR_HANDLE            getRTVHandle();
	ID3D12Resource* vertexBuffer;             // a default buffer in GPU memory that we will load vertex data for our triangle into
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

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
	ComPtr<ID3D12Fence> fence[frameBufferCount]; 
	ComPtr<ID3D12DescriptorHeap>cbvDescriptorHeap[frameBufferCount];// an object that is locked while our command list is being executed by the gpu
	ComPtr<ID3D12Resource> constantBufferUploadHeap[frameBufferCount];//#######################

	HANDLE fenceEvent;                                          // a handle to an event when our fence is unlocked by the gpu
	UINT64 fenceValue[frameBufferCount];  // this value is incremented each frame. each fence will have its own value
	UINT8* cbvGPUAddress[frameBufferCount];//####################
	int frameIndex;                                             // current rtv we are on
	int rtvDescriptorSize;// size of the rtv descriptor on the device (all front and back buffers will be the same size)
	ConstantBuffer cbvData;
	ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state
	ID3D12RootSignature* rootSignature;       // root signature defines data shaders will access
	D3D12_VIEWPORT viewport;                  // area that output from rasterizer will be stretched to.
	D3D12_RECT scissorRect;                   // the area to draw in. pixels outside that area will not be drawn onto


	bool InitD3D();
	bool InitializeWindow(HINSTANCE hInstance,
		int ShowWnd,
		bool fullscreen, LPCWSTR windowName);

	void UpdatePipeline();

};