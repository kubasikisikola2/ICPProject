#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <algorithm>
#include "Camera.hpp"
#include "Config.hpp"

enum class PlayerMode
{
    FreeFly,
    FirstPerson
};

class Player
{
private:
    glm::vec3 position{ 0.0f, 2.0f, 10.0f };
    glm::vec3 velocity{ 0.0f };

    float floor_height = 0.0f;
    float eye_height = 2.5f;

    float max_speed = 10.0f;
    float acceleration = 60.0f;
    float drag = 12.0f;

    float gravity = 25.0f;
    float jump_speed = 8.0f;
    float vertical_velocity = 0.0f;
    bool grounded = false;

    float room_min_x = -(ROOM_SIZE / 2 - 0.5f);
    float room_max_x = (ROOM_SIZE / 2 - 0.5f);
    float room_min_z = 0.5f;
    float room_max_z = (ROOM_SIZE / 2 - 0.5f);

    float collision_radius = 0.3f;

    PlayerMode mode = PlayerMode::FirstPerson;

public:
    void set_mode(PlayerMode new_mode);
    PlayerMode get_mode() const;
    void set_floor_height(float h);
    glm::vec3 get_position() const;
    glm::vec3 get_eye_position() const;
    void update(GLFWwindow* window, float dt, Camera& camera);
};