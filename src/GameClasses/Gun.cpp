#include "GameClasses/Gun.hpp"

void Gun::update(float dt,
    const glm::vec3& cam_pos,
    const glm::vec3& cam_front,
    const glm::vec3& cam_up,
    const glm::vec3& cam_right)
{
    //position the gun relative to camera
    glm::vec3 gunOffset = cam_right * 0.25f + cam_up * -0.20f + cam_front * 0.20f;
    position = cam_pos + gunOffset;

    //recoil animation
    if (is_recoiling) {
        recoil_angle += recoil_speed * dt;
        if (recoil_angle >= max_recoil_angle) {
            recoil_angle = max_recoil_angle;
            is_recoiling = false;
        }
    }
    else if (recoil_angle > 0.0f) {
        recoil_angle -= recoil_return_speed * dt;
        if (recoil_angle < 0.0f) {
            recoil_angle = 0.0f;
            shooting = false;
        } 
    }

    //base gun orientation from camera
    glm::mat4 gun_orient(1.0f);
    gun_orient[0] = glm::vec4(cam_right, 0.0f);   // local X
    gun_orient[1] = glm::vec4(cam_up, 0.0f);      // local Y
    gun_orient[2] = glm::vec4(-cam_front, 0.0f);  // local Z

    //recoil rotation 
    glm::mat4 recoil_rot = glm::rotate(glm::mat4(1.0f), glm::radians(recoil_angle), glm::vec3(1, 0, 0));

    //align model forward to camera
    glm::mat4 gun_correction = glm::rotate(glm::mat4(1.0f),
        glm::radians(-90.0f),
        glm::vec3(0, 1, 0));

    //combine matrices
    glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), position) *
        gun_orient *
        recoil_rot *
        gun_correction;
    model->setModelMatrix(model_matrix);


    if (muzzle_flash_active) {
        muzzle_flash_timer -= dt;
        if (muzzle_flash_timer <= 0.0f) {
            muzzle_flash_active = false;
            muzzle_flash_timer = 0.0f;
        }
    }

    //muzzle flash
    glm::mat4 flash_local_offset =
        glm::translate(glm::mat4(1.0f),
            glm::vec3(muzzle_offset_x, muzzle_offset_y, muzzle_offset_z));

    glm::mat4 flash_local_rotation =
        glm::mat4(1.0f);

    glm::mat4 flash_local_scale =
        glm::scale(glm::mat4(1.0f), glm::vec3(muzzle_flash_scale));

    muzzle_flash_matrix =
        model_matrix *
        flash_local_offset *
        flash_local_rotation *
        flash_local_scale;

    flash_model->setModelMatrix(muzzle_flash_matrix);
}

glm::mat4& Gun::get_muzzle_flash_matrix(){
    return muzzle_flash_matrix;
}

void Gun::set_model(std::shared_ptr<Model> model){
    this->model = model;
}

void Gun::shoot() {
    is_recoiling = true;
    shooting = true;
    muzzle_flash_active = true;
    muzzle_flash_timer = muzzle_flash_duration;
}

bool Gun::is_shooting(){
    return shooting;
}

bool Gun::is_muzzle_flash_active() {
    return muzzle_flash_active;
}

void Gun::set_flash_model(std::shared_ptr<Model> model) {
    flash_model = model;
}