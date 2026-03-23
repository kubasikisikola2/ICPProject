#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 Front{ 0.0f, 0.0f, -1.0f };
    glm::vec3 Right{ 1.0f, 0.0f, 0.0f };
    glm::vec3 Up{ 0.0f, 1.0f, 0.0f };

    float Yaw = -90.0f;
    float Pitch = 0.0f;
    float mouse_sensitivity = 0.25f;

    glm::vec3 world_up{ 0.0f, 1.0f, 0.0f };

    Camera()
    {
        updateVectors();
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= mouse_sensitivity;
        yoffset *= mouse_sensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        updateVectors();
    }

    glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void setPosition(const glm::vec3& pos)
    {
        Position = pos;
    }

private:
    void updateVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, world_up));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};