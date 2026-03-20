#pragma once

#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

class Camera
{
public:

    // Camera Attributes
    glm::vec3 Position{0,0,10};
    glm::vec3 Front{};
    glm::vec3 Right{};
    glm::vec3 Up{}; // camera local UP vector

    GLfloat Yaw = -90.0f;
    GLfloat Pitch = 0.0f;
    GLfloat Roll = 0.0f;


    // Camera options
    GLfloat max_speed = 40.0f;
    GLfloat mouse_sensitivity = 0.25f;
    GLfloat stop_speed_tresh = 5.0f;

    glm::vec3 world_up{ 0.0f, 1.0f, 0.0f };

    glm::vec3 velocity = glm::vec3(0.0f);
    float acceleration = 100.0f;
    float drag = 78.0f;
    
    Camera(){
        // Default constructor initializes camera's position and orientation
        this->updateCameraVectors();
    }

    Camera(glm::vec3 position) :Position(position)
    {
        this->Up = glm::vec3(0.0f, 1.0f, 0.0f);
        // initialization of the camera reference system
        this->updateCameraVectors();
    }

    void ProcessInput(GLFWwindow* window, GLfloat deltaTime)
    {
        glm::vec3 direction{ 0 };
        bool keyPressed = false;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            direction += Front;
            keyPressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            direction -= Front;
            keyPressed = true;
        }
            

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            direction -= Right;
            keyPressed = true;
        }
            

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            direction += Right;
            keyPressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            direction += Up;
            keyPressed = true;
        }
            

        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            direction -= Up;
            keyPressed = true;
        }

        if (glm::length(direction) > 0.0001f)
            direction = glm::normalize(direction);
        else
            direction = glm::vec3(0);

        glm::vec3 accelerationTemp = direction * acceleration;

        // Apply drag and accel
        accelerationTemp -= velocity * drag * deltaTime;
        velocity += accelerationTemp * deltaTime;

        // Clamp max speed or when speed is low stop
        GLfloat absoluteVelocity = glm::length(velocity);
        if (absoluteVelocity > max_speed)
            velocity = glm::normalize(velocity) * max_speed;
        else if (absoluteVelocity <= stop_speed_tresh && !keyPressed){
            velocity = glm::vec3(0);
        }

        Position += velocity * deltaTime;
    }

    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE)
    {
        xoffset *= this->mouse_sensitivity;
        yoffset *= this->mouse_sensitivity;

        this->Yaw += xoffset;
        this->Pitch += yoffset;

        if (constraintPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        this->updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }
private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f)));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};