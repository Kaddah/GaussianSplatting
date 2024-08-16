#include "GaussianRenderer.h"
#include "Camera.h" // Include the Camera header
#include "Window.h"
#include <DxException.h>
#include <iostream>

// #12 Create Quad generator parameters - MH
std::vector<uint32_t> quadIndices;

float quadSize = 0.1f;

GaussianRenderer::GaussianRenderer(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance,
                                   int nShowCmd, const std::vector<Vertex>& vertices)
    : Window(WindowName, width, height, fullScreen, hInstance, nShowCmd)
    , m_vertices(vertices)
    
{
}

void GaussianRenderer::draw()
{
  const auto rtvHandle = getRTVHandle();
  // set the render target for the output merger stage (the output of the pipeline)
  commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

  // Clear the render target by using the ClearRenderTargetView command
  const float clearColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

  // draw triangle
  commandList->SetGraphicsRootSignature(rootSignature);
  commandList->RSSetViewports(1, &viewport);
  commandList->RSSetScissorRects(1, &scissorRect);

     // Bind the vertex buffer and index buffer to the pipeline
  commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
  commandList->IASetIndexBuffer(&indexBufferView);


  // Set the primitive topology to point list
  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

  commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[frameIndex]->GetGPUVirtualAddress());
  // commandList->DrawInstanced(getQuadVertices().size() / 4, 1, 0, 0); // draw 3 vertices (draw the triangle)

  // UINT indexCount = 10;
  //  Draw the points using the index buffer
  commandList->DrawIndexedInstanced(indexBufferView.SizeInBytes / sizeof(uint32_t), // Number of indices to draw
                                    1,                                              // Number of instances to draw
                                    0,                                              // Start index location
                                    0,                                              // Base vertex location
                                    0                                               // Start instance location
  );
}

void GaussianRenderer::drawUI()
{
  ImGui::Begin("Gaussian Splatting");
  ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera->getCameraPos().x, camera->getCameraPos().y,
              camera->getCameraPos().z);

  float cameraSpeed = camera->getCameraSpeed();
  if (ImGui::SliderFloat("Camera Speed", &cameraSpeed, 1.0f, 10.0f))
  {
    camera->setCameraSpeed(cameraSpeed);
  }

  if (ImGui::Button("Reset Camera"))
  {
    camera->setCameraPos(glm::vec3(0.0f, 0.0f, 5.0f));
    camera->setCameraFront(glm::vec3(0.0f, 0.0f, -1.0f));
    camera->setCameraUp(glm::vec3(0.0f, 1.0f, 0.0f));
  }
  ImGui::Spacing();
  ImGui::Text("Object Position: (%.2f, %.2f, %.2f)", camera->getAlphaX(), camera->getAlphaY(), camera->getAlphaZ());
  if (ImGui::Button("Reset Object"))
  {
    camera->setAlphaX(0.0f);
    camera->setAlphaY(0.0f);
    camera->setAlphaZ(0.0f);
  }
  ImGui::End();
}


std::vector<VertexPos> GaussianRenderer::prepareIndices(const std::vector<Vertex>& vertices)
{
  // #xx get indices from ply file
  //  Assuming vertices is a member of GaussianRenderer that contains the Vertex data
  std::vector<VertexPos> indices;
  indices.reserve(vertices.size());

  for (size_t i = 0; i < vertices.size(); ++i)
  {
    VertexPos vp;
    vp.position = vertices[i].pos;          // Copy the position
    vp.index    = static_cast<uint32_t>(i); // Set the index
    indices.push_back(vp);
  }

  return indices;
}

// Setter method implementation
void GaussianRenderer::setVertices(const std::vector<Vertex>& vertices)
{
  m_vertices = vertices;
}
// Implementation of the getter method
const std::vector<Vertex>& GaussianRenderer::getVertices() const
{
  return m_vertices;
}

