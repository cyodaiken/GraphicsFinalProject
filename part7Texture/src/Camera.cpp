/** @file Camera.cpp
 * Code from class
 */


#include "Camera.hpp"

#include "glm/gtx/transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include <iostream>

Camera& Camera::Instance(){
    static Camera* instance = new Camera();
    return *instance;
}

void Camera::MouseLook(int mouseX, int mouseY){
    // Record our new position as a vector
    glm::vec2 newMousePosition(mouseX, mouseY);

    static bool initialLook = true;
    if (initialLook == true) {
        initialLook = false;
        m_oldMousePosition = newMousePosition;
    }
    // Detect how much the mouse has moved since
    // the last time
    glm::vec2 mouseDelta = m_oldMousePosition - newMousePosition;
    // mouse sensitivity
    mouseDelta.x *= 0.2f;
    mouseDelta.y *= 0.2f;

    m_viewDirection = glm::rotate(m_viewDirection, glm::radians(mouseDelta.x), m_upVector);

    // Compute rightVector
    glm::vec3 rightVector = glm::cross(m_viewDirection, m_upVector);
    m_viewDirection = glm::rotate(m_viewDirection, glm::radians(mouseDelta.y), rightVector);

    // Update our old position after we have made changes 
    m_oldMousePosition = newMousePosition;
}

void Camera::MoveForward(float speed){

    // Move the camera in the view direction
    glm::vec3 direction(m_viewDirection.x,m_viewDirection.y,m_viewDirection.z);
    // Move in the speed
    direction = direction * speed;
    // Update the position
    m_eyePosition += direction;
    //m_eyePosition.z -= speed;
}

void Camera::MoveBackward(float speed){

    // Move the camera in the view direction
    glm::vec3 direction(m_viewDirection.x,m_viewDirection.y,m_viewDirection.z);
    // Move in the speed
    direction = direction * speed;
    // Update the position
    m_eyePosition -= direction;
}

void Camera::MoveLeft(float speed){

    // Compute the rightVector
    glm::vec3 rightVector = glm::cross(m_viewDirection, m_upVector);
    // Move in the direction along the
    glm::vec3 direction(rightVector.x,0.0,rightVector.z);
    // Update the speed
    direction = direction * speed;
    // Update the eye position
    m_eyePosition -= direction;
}

void Camera::MoveRight(float speed){
    // Compute the rightVector
    glm::vec3 rightVector = glm::cross(m_viewDirection, m_upVector);
    // Move in the direction along the
    glm::vec3 direction(rightVector.x,0.0,rightVector.z);
    // Update the speed
    direction = direction * speed;
    // Update the eye position
    m_eyePosition += direction;
}

void Camera::MoveUp(float speed){
    m_eyePosition.y += speed;
}

void Camera::MoveDown(float speed){
    m_eyePosition.y -= speed;
}

float Camera::GetEyeXPosition(){
    return m_eyePosition.x;
}

float Camera::GetEyeYPosition(){
    return m_eyePosition.y;
}

float Camera::GetEyeZPosition(){
    return m_eyePosition.z;
}

float Camera::GetViewXDirection(){
    return m_viewDirection.x;
}

float Camera::GetViewYDirection(){
    return m_viewDirection.y;
}

float Camera::GetViewZDirection(){
    return m_viewDirection.z;
}

Camera::Camera(){
    std::cout << "Created a Single Camera!\n";
	// Position us at the origin.
    m_eyePosition = glm::vec3(2.0f,0.0f, 0.0f);
	// Looking down along the z-axis initially.
	// Remember, this is negative because we are looking 'into' the scene.
    m_viewDirection = glm::vec3(0.0f,0.0f, -1.0f);
	// For now--our upVector always points up along the y-axis
    m_upVector = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::GetWorldToViewmatrix() const{
    // Think about the second argument and why that is
    // setup as it is.
    return glm::lookAt( m_eyePosition,
                        m_eyePosition + m_viewDirection,
                        m_upVector);
}
