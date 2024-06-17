#pragma once


#include <stdlib.h>
#include <limits.h>
#include <windef.h>
#include <glm/glm.hpp>

//float alphaX = 0.0f;
//float alphaY = 0.0f;
//float alphaZ = 0.0f;
//
//const float mouseSensX = 0.005f;
//const float mouseSensY = 0.005f;
//
//// Store previous mouse position
////static POINT prevMousePosRotation        = {0, 0};
////static POINT prevMousePosCameraDirection = {0, 0};
//
//// Camera position and movement variables
//glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
//glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
//glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
//
//float cameraSpeed = 1.0f;  // Camera speed in meters per second
//float fov         = 45.0f; // Initial zoom level (FOV)
//
//float nearPlane = 0.1f;
//float farPlane  = 100.0f;
//
//float yaw   = -90.0f; // Initialize to face towards negative z-axis
//float pitch = 0.0f;
// custom definitions
const float ERROR_EPSILON = 1e-4;

// returns a random float between 0 and 1.
inline float random_float() {
#if defined ( _WIN64 )
    unsigned int n;
    return static_cast<float>( n ) / ( static_cast<double>( UINT_MAX ) + 1.0 );
#else
    return drand48();
#endif

}