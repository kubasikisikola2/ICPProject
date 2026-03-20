#include "Player.hpp"

void Player::set_mode(PlayerMode newMode){
    mode = newMode;
    velocity = glm::vec3(0.0f);
    vertical_velocity = 0.0f;

    if (mode == PlayerMode::FirstPerson) {
        if (position.y < floor_height) {
            position.y = floor_height;
        }
    }
}

    PlayerMode Player::get_mode() const{
        return mode;
    }

    void Player::set_floor_height(float h){
        floor_height = h;
    }

    glm::vec3 Player::get_position() const{
        return position;
    }

    glm::vec3 Player::get_eye_position() const
    {
        if (mode == PlayerMode::FirstPerson) {
            return position + glm::vec3(0.0f, eye_height, 0.0f);
        }
        return position;
    }

    void update(GLFWwindow* window, float dt, Camera& camera)
    {
        glm::vec3 moveDir(0.0f);
        bool moving = false;

        if (mode == PlayerMode::FreeFly)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { moveDir += camera.Front; moving = true; }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { moveDir -= camera.Front; moving = true; }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { moveDir -= camera.Right; moving = true; }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { moveDir += camera.Right; moving = true; }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { moveDir += camera.Up; moving = true; }
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) { moveDir -= camera.Up; moving = true; }

            if (glm::length(moveDir) > 0.0001f)
                moveDir = glm::normalize(moveDir);

            glm::vec3 accel = moveDir * acceleration;
            accel -= velocity * drag;
            velocity += accel * dt;

            if (glm::length(velocity) > max_speed)
                velocity = glm::normalize(velocity) * max_speed;

            if (!moving && glm::length(velocity) < 0.05f)
                velocity = glm::vec3(0.0f);

            position += velocity * dt;
        }
        else
        {
            glm::vec3 flatFront = glm::vec3(camera.Front.x, 0.0f, camera.Front.z);
            glm::vec3 flatRight = glm::vec3(camera.Right.x, 0.0f, camera.Right.z);

            if (glm::length(flatFront) > 0.0001f) flatFront = glm::normalize(flatFront);
            if (glm::length(flatRight) > 0.0001f) flatRight = glm::normalize(flatRight);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { moveDir += flatFront; moving = true; }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { moveDir -= flatFront; moving = true; }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { moveDir -= flatRight; moving = true; }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { moveDir += flatRight; moving = true; }

            if (glm::length(moveDir) > 0.0001f)
                moveDir = glm::normalize(moveDir);

            glm::vec3 horizontalVelocity = glm::vec3(velocity.x, 0.0f, velocity.z);
            glm::vec3 accel = moveDir * acceleration;
            accel -= horizontalVelocity * drag;
            horizontalVelocity += accel * dt;

            if (glm::length(horizontalVelocity) > max_speed)
                horizontalVelocity = glm::normalize(horizontalVelocity) * max_speed;

            if (!moving && glm::length(horizontalVelocity) < 0.05f)
                horizontalVelocity = glm::vec3(0.0f);

            velocity.x = horizontalVelocity.x;
            velocity.z = horizontalVelocity.z;

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && grounded) {
                vertical_velocity = jump_speed;
                grounded = false;
            }

            vertical_velocity -= gravity * dt;

            position.x += velocity.x * dt;
            position.z += velocity.z * dt;
            position.y += vertical_velocity * dt;

            if (position.y <= floor_height) {
                position.y = floor_height;
                vertical_velocity = 0.0f;
                grounded = true;
            }
        }

        camera.setPosition(getEyePosition());
}