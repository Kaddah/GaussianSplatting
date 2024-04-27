#include "GaussianRenderer.h"
#include "Window.h"


GaussianRenderer::GaussianRenderer(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd) : Window(WindowName, width, height, fullScreen, hInstance, nShowCmd)
{

}

void GaussianRenderer::draw()
{
    const auto rtvHandle = getRTVHandle();
    // set the render target for the output merger stage (the output of the pipeline)
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear the render target by using the ClearRenderTargetView command
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // draw triangle
    commandList->SetGraphicsRootSignature(rootSignature);                     
    commandList->RSSetViewports(1, &viewport);                               
    commandList->RSSetScissorRects(1, &scissorRect);                          
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);                 
    commandList->DrawInstanced(3, 1, 0, 0);                                   //draw 3 vertices (draw the triangle)
}

void GaussianRenderer::prepareTriangle()
{
}
