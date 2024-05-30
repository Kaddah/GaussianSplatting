#include "GaussianRenderer.h"
#include "Window.h"
#include <iostream>
#include <GeoGenerator.h>
#include <DxException.h>



//#12 Create Quad generator parameters - MH 
//std::vector<Vertex> quadVertices;
std::vector<uint32_t> quadIndices;

GeoGenerator quadGen;

//std::vector<Vertex> m_quadVertices;
float quadSize = 0.1f;


GaussianRenderer::GaussianRenderer(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd, const std::vector<Vertex>& vertices, const std::vector<Vertex>& quads)
    : Window(WindowName, width, height, fullScreen, hInstance, nShowCmd), m_vertices(vertices), m_quads(vertices)
{

}

void GaussianRenderer::draw()
{
    const auto rtvHandle = getRTVHandle();
    // set the render target for the output merger stage (the output of the pipeline)
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear the render target by using the ClearRenderTargetView command
    const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // draw triangle
    commandList->SetGraphicsRootSignature(rootSignature);                     
    commandList->RSSetViewports(1, &viewport);                               
    commandList->RSSetScissorRects(1, &scissorRect);                          
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[frameIndex]->GetGPUVirtualAddress());  
    
    commandList->DrawInstanced(getQuadVertices().size(), 1, 0, 0);                                   //draw 3 vertices (draw the triangle)
}
    
std::vector<Vertex>  GaussianRenderer::prepareTriangle(){
    m_quads.clear();
    //#12 Access baseVertices from PLY file
    const std::vector<Vertex>m_Vertices = getVertices();

       //#12 Generate quad cloud mesh - MH
    std::vector <GeoGenerator::MeshData> quads = quadGen.GenerateQuadsForVertices(m_vertices, quadSize);

    uint32_t currentIndexOffset = 0;
    for (const auto& quad : quads) {
        m_quads.insert(m_quads.end(), quad.vertices.begin(), quad.vertices.end());
        for (auto index : quad.indices) {
            quadIndices.push_back(index + currentIndexOffset);
        }
        currentIndexOffset += quad.vertices.size(); // Update offset for next quad
    }
  return m_quads;
}

// Setter method implementation
void GaussianRenderer::setVertices(const std::vector<Vertex>& vertices) {
    m_vertices = vertices;
}
// Implementation of the getter method
const std::vector<Vertex>& GaussianRenderer::getVertices() const {
    return m_vertices;
}

// Setter method implementation
void GaussianRenderer::setQuadVertices(const std::vector<Vertex>& vertices) {
    m_quadVertices = vertices;
}
// Implementation of the getter method
const std::vector<Vertex>& GaussianRenderer::getQuadVertices()  {
    return m_quads;
}
