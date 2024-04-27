#include "GaussianRenderer.h"
#include "Window.h"


GaussianRenderer::GaussianRenderer(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd) : Window(WindowName, width, height, fullScreen, hInstance, nShowCmd)
{

}

void GaussianRenderer::draw()
{
	//code meyer:
	//getcommandlist,rtv handle, dsv handle
	//danach kommt ganz viel commandlist shit und clear color::
    const auto rtvHandle = getRTVHandle();
    const auto dsvHandle = getDSVHandle();
    // set the render target for the output merger stage (the output of the pipeline)
    
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr); //&dsvHandle

    // Clear the render target by using the ClearRenderTargetView command
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    //commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); //meby

    // draw triangle
    commandList->SetGraphicsRootSignature(rootSignature);                     // set the root signature
    commandList->RSSetViewports(1, &viewport);                                // set the viewports
    commandList->RSSetScissorRects(1, &scissorRect);                          // set the scissor rects
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);                 // set the vertex buffer (using the vertex buffer view)
    commandList->DrawInstanced(3, 1, 0, 0);                                   // finally draw 3 vertices (draw the triangle)
}

void GaussianRenderer::prepareTriangle()
{
}
