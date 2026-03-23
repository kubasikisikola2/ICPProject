#include <memory>
#include "Model.hpp"

class Gun {
public:
    float muzzle_offset_x = -1.655f;
    float muzzle_offset_y = -0.426f;
    float muzzle_offset_z = 0.016f;

    void update(float dt, const glm::vec3& cam_pos, const glm::vec3& cam_front,
        const glm::vec3& cam_up, const glm::vec3& cam_right);
    void shoot();
    void set_model(std::shared_ptr<Model> model);
    void set_flash_model(std::shared_ptr<Model> model);
    bool is_shooting();
    glm::mat4& get_muzzle_flash_matrix();
    bool is_muzzle_flash_active();

private:
    std::shared_ptr<Model> model;
    std::shared_ptr<Model> flash_model;

    glm::vec3 position;
    glm::mat4 orientation;

    float recoil_angle = 0.0f;
    float recoil_speed = 60.0f;
    float recoil_return_speed = 40.0f;
    bool is_recoiling = false;
    bool shooting = false;
    float max_recoil_angle = 5.0f;

    bool muzzle_flash_active = false;
    float muzzle_flash_timer = 0.0f;
    float muzzle_flash_duration = 0.04f; 
    float muzzle_flash_scale = 1.0f;
    glm::mat4 muzzle_flash_matrix{ 1.0f };
};

