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
    
    commandList->DrawInstanced(getQuadVertices().size(), 1, 0, 0);                                   //draw 3 vertices (draw the triangle)
}
    

std::vector<Vertex>  GaussianRenderer::prepareTriangle(){
    m_quads.clear();
    //#12 Access baseVertices from PLY file
    const std::vector<Vertex>m_Vertices = getVertices();
    // #10 check import success - MH
    // Plott vertices for debuging
    //std::cout << "Number of imported vertices: " << m_vertices.size() << std::endl;
    //for (size_t i = 0; i < m_vertices.size(); ++i) {
    //  const Vertex& vertex = m_vertices[i];
    //  std::cout << "Vertex " << i << ": " << std::endl;
    //  std::cout << "  Position: (" << vertex.pos.x << ", " << vertex.pos.y << ", " << vertex.pos.z << ")" << std::endl;
    //  std::cout << "  Normale: (" << vertex.normal.x << ", " << vertex.normal.y << ", " << vertex.normal.z << ")" << std::endl;
    //  std::cout << "  Color: (" << static_cast<int>(vertex.color.r) << ", " << static_cast<int>(vertex.color.g) << ", " << static_cast<int>(vertex.color.b) << ")" << std::endl;
    // }

       //#12 Generate quad cloud mesh - MH
    std::vector <GeoGenerator::MeshData> quads = quadGen.GenerateQuadsForVertices(m_vertices, quadSize);

    //#12 Check if quads are calculated correctly - MH
   /* for (size_t i = 0; i < quads.size(); ++i) {
        std::cout << "Quads for Vertex " << i << ":" << std::endl;
        const auto& meshData = quads[i];
        for (size_t j = 0; j < meshData.vertices.size(); ++j) {
            const Vertex& quadVertex = meshData.vertices[j];
            std::cout << "  Quad Vertex " << j << ": " << std::endl;
            std::cout << "    Position: (" << quadVertex.pos.x << ", " << quadVertex.pos.y << ", " << quadVertex.pos.z << ")" << std::endl;
            std::cout << "    Normal: (" << quadVertex.normal.x << ", " << quadVertex.normal.y << ", " << quadVertex.normal.z << ")" << std::endl;
            std::cout << "    Color: (" << static_cast<int>(quadVertex.color.r) << ", " << static_cast<int>(quadVertex.color.g) << ", " << static_cast<int>(quadVertex.color.b) << ")" << std::endl;
        }
    }*/

    uint32_t currentIndexOffset = 0;
    for (const auto& quad : quads) {
        m_quads.insert(m_quads.end(), quad.vertices.begin(), quad.vertices.end());
        for (auto index : quad.indices) {
            quadIndices.push_back(index + currentIndexOffset);
        }
        currentIndexOffset += quad.vertices.size(); // Update offset for next quad
    }

//#12 CHECK if quads are created
//    std::cout << "Quad Vertices: \n";
//for (size_t i = 0; i < m_quads.size(); ++i) {
//    const Vertex& v = m_quads[i];
//    std::cout << "Vertex " << i << ":\n"
//        << "  Position: (" << v.pos.x << ", " << v.pos.y << ", " << v.pos.z << ")\n"
//        << "  Normal: (" << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << ")\n"
//        << "  Color: (" << static_cast<int>(v.color.r) << ", "
//        << static_cast<int>(v.color.g) << ", "
//        << static_cast<int>(v.color.b) << ")\n";
//}

  //std::cout << "Quad Vertices:\n" << m_quads.data();

  //std::cout << "Quad Indices: \n";
  //for (size_t i = 0; i < quadIndices.size(); ++i) {
  //    std::cout << quadIndices[i];
  //    if ((i + 1) % 3 == 0) // Assuming triangles, print a newline every three indices
  //        std::cout << "\n";
  //    else
  //        std::cout << ", ";
  //}
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
