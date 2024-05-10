#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <D3Dcompiler.h>
#include <wrl/client.h>
#include <Vertex.h>
#include<vector.h>


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
		//camera height is fixed at 2.0f
		// Update view matrix based on new camera position
		cameraViewMat = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(cameraTarget), glm::vec3(cameraUp));

		// Apply transformations to object
		// For example:
		// Rotate the object over time
		float deltaTime = 0.016f; // example delta time
		float rotationSpeed = glm::radians(30.0f); // example rotation speed (30 degrees per second)
		objectWorldMat = glm::rotate(objectWorldMat, rotationSpeed * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f)); // rotating around Y-axis

		// Combine view and projection matrix
		glm::mat4 wvpMat = cameraProjMat * cameraViewMat * objectWorldMat; // Model-View-Projection matrix
		///TODO transponieren, im constant buffer speichern(Struct) und ann cbvGpuadress mappen.
	}
};
