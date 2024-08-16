#pragma once

// Define GLM_ENABLE_EXPERIMENTAL before including GLM headers
#define GLM_ENABLE_EXPERIMENTAL
#include <Windows.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Camera
{
public:
  Camera();

  void UpdatePosition();
  void UpdateDirection();
  void UpdateRotationFromMouse();
  void OrbitalCamera();
  void ZoomCamera(int delta);
  void InitializeMousePosition();
  

  glm::vec3 getPosition() const
  {
    return cameraPos;
  }
  glm::vec3 getFront() const
  {
    return cameraFront;
  }
  glm::vec3 getUp() const
  {
    return cameraUp;
  }
  glm::mat4 getViewMatrix() const
  {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  }
  glm::mat4 getOrbitalViewMatrix() const
  {
    return glm::lookAt(cameraPos, cameraTarget, cameraUp);
  }
  glm::mat4 updateViewMatrix();

  void setWindowDimensions(int width, int height)
  {
    _width  = width;
    _height = height;
  }
  void setOrbiCam(bool value)
  {
    orbiCam = value;
  }
  bool getOrbiCam() const
  {
    return orbiCam;
  }

  float getAlphaX() const
  {
    return alphaX;
  }
  void setAlphaX(float value)
  {
    alphaX = value;
  }

  float getAlphaY() const
  {
    return alphaY;
  }
  void setAlphaY(float value)
  {
    alphaY = value;
  }

  float getAlphaZ() const
  {
    return alphaZ;
  }
  void setAlphaZ(float value)
  {
    alphaZ = value;
  }

  glm::vec3 getCameraPos() const
  {
    return cameraPos;
  }
  void setCameraPos(const glm::vec3& value)
  {
    cameraPos = value;
  }

  glm::vec3 getCameraFront() const
  {
    return cameraFront;
  }
  void setCameraFront(const glm::vec3& value)
  {
    cameraFront = value;
  }

  glm::vec3 getCameraUp() const
  {
    return cameraUp;
  }
  void setCameraUp(const glm::vec3& value)
  {
    cameraUp = value;
  }

  glm::vec3 getCameraTarget() const
  {
    return cameraTarget;
  }

  void setCameraTarget(const glm::vec3& value)
  {
    cameraTarget = value;
  }

  float getCameraSpeed() const
  {
    return cameraSpeed;
  }
  void setCameraSpeed(float value)
  {
    cameraSpeed = value;
  }

  float getFov() const
  {
    return fov;
  }
  void setFov(float value)
  {
    fov = value;
  }

  float getNearPlane() const
  {
    return nearPlane;
  }
  void setNearPlane(float value)
  {
    nearPlane = value;
  }

  float getFarPlane() const
  {
    return farPlane;
  }
  void setFarPlane(float value)
  {
    farPlane = value;
  }
  
  float getPhi() const
  {
    return phi;
  }

  void setPhi(float value)
  {
    phi = value;
  }
     

  float getTheta() const
  {
    return theta;
  }

  void setTheta(float value)
  {
      theta = value;
  }

  float getRadius() const
  {
    return radius;
  }

  void setRadius(float value)
  {
    radius = value;
  }





 

private:
  bool orbiCam = false;

  POINT prevMousePosCameraDirection = {0, 0};
  POINT prevMousePosRotation        = {0, 0};
  POINT prevMousePosCameraFocus     = {0, 0};

  float alphaX = 0.0f;
  float alphaY = 0.0f;
  float alphaZ = 0.0f;

  float theta  = 0.0f;
  float phi    = 0.0f;
  float radius = 5.0f;

  const float mouseSensX = 0.005f;
  const float mouseSensY = 0.005f;

  const float targetSensX = 0.05f;
  const float targetSensY = 0.05f;


  glm::vec3 cameraPos    = glm::vec3(0.0f, 0.0f, 5.0f);
  glm::vec3 cameraFront  = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp     = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

  float cameraSpeed = 1.0f;
  float fov         = 45.0f;

  float nearPlane = 0.1f;
  float farPlane  = 100.0f;

  float yaw   = -90.0f;
  float pitch = 0.0f;

  int _width  = 0;
  int _height = 0;

  std::chrono::high_resolution_clock::time_point before;
};
