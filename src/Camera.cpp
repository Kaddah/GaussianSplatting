#include "Camera.h"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <windows.h>
#include "imgui.h"

Camera::Camera()
{
  InitializeMousePosition();
}

void Camera::UpdatePosition()
{
  auto  now    = std::chrono::high_resolution_clock::now();
  float deltaS = std::chrono::duration_cast<std::chrono::nanoseconds>(now - before).count() / 1e9f;
  before       = now;
  if (GetAsyncKeyState('W') & 0x8000)
  {
    cameraPos += cameraSpeed * cameraFront * deltaS;
  }
  if (GetAsyncKeyState('S') & 0x8000)
  {
    cameraPos -= cameraSpeed * cameraFront * deltaS;
  }
  if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
  {
    cameraPos -= cameraSpeed * cameraUp * deltaS;
  }
  if (GetAsyncKeyState(VK_SPACE) & 0x8000)
  {
    cameraPos += cameraSpeed * cameraUp * deltaS;
  }

  const glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
  if (GetAsyncKeyState('A') & 0x8000)
  {
    cameraPos -= cameraRight * cameraSpeed * deltaS;
  }
  if (GetAsyncKeyState('D') & 0x8000)
  {
    cameraPos += cameraRight * cameraSpeed * deltaS;
  }
}

void Camera::UpdateDirection()
{
  if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
  {
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);

    int deltaX = currentMousePos.x - prevMousePosCameraDirection.x;
    int deltaY = currentMousePos.y - prevMousePosCameraDirection.y;

    float sensitivity = 0.05f;
    yaw += deltaX * sensitivity;
    pitch -= deltaY * sensitivity;

    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = 0.0f;
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    cameraUp              = glm::normalize(glm::cross(cameraRight, cameraFront));

    prevMousePosCameraDirection = currentMousePos;
  }
  else
  {
    GetCursorPos(&prevMousePosCameraDirection);
  }
}

void Camera::UpdateRotationFromMouse()
{
  if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive())
  {
    return;
  }
  if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
  {
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);

    int deltaX = currentMousePos.x - prevMousePosRotation.x;
    int deltaY = currentMousePos.y - prevMousePosRotation.y;

    alphaY += deltaX * mouseSensX;
    alphaX += deltaY * mouseSensY;

    prevMousePosRotation = currentMousePos;
  }
  else
  {
    GetCursorPos(&prevMousePosRotation);
  }
}

void Camera::InitializeMousePosition()
{
  POINT initialMousePos;
  GetCursorPos(&initialMousePos);
  prevMousePosRotation        = initialMousePos;
  prevMousePosCameraDirection = initialMousePos;
}

void Camera::ZoomCamera(int delta)
{
  const float zoomSpeed = 0.006f; // Adjust the speed of zooming
  radius -= delta * zoomSpeed;

  // Clamp the radius to prevent too much zooming in or out
  if (radius < 1.0f)
    radius = 1.0f;
  if (radius > 50.0f)
    radius = 50.0f;
}

void Camera::OrbitalCamera()
{
  if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
  {
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);

    int deltaX = currentMousePos.x - prevMousePosCameraFocus.x;
    int deltaY = currentMousePos.y - prevMousePosCameraFocus.y;

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) // Shift key is pressed
    {
      // Update camera target position
      cameraTarget.x += deltaX * targetSensX;
      cameraTarget.y += deltaY * targetSensY;
    }
    else
    {
      theta += deltaX * mouseSensX;
      phi -= deltaY * mouseSensY;

      if (phi > glm::radians(89.0f))
        phi = glm::radians(89.0f);
      if (phi < glm::radians(-89.0f))
        phi = glm::radians(-89.0f);
    }
    prevMousePosCameraFocus = currentMousePos;
  }
  else
  {
    GetCursorPos(&prevMousePosCameraFocus);
  }

  cameraPos.x = cameraTarget.x + radius * cos(phi) * cos(theta);
  cameraPos.y = cameraTarget.y + radius * sin(phi);
  cameraPos.z = cameraTarget.z + radius * cos(phi) * sin(theta);
}

glm::mat4 Camera::updateViewMatrix()
{
  if (orbiCam)
  {
    return glm::lookAt(cameraPos, cameraTarget, cameraUp);
  }
  else
  {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  }
}