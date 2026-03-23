
#pragma once 
#include "Model.hpp"

class Target{
public:
    bool target_hit(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
    void set_model(std::shared_ptr<Model> model);
    void set_hitbox_offset(glm::vec3 hitbox_offset);
    glm::vec3 get_center();
    std::shared_ptr<Model> get_model();
private:
    std::shared_ptr<Model> model;
    float sphereRadius = 0.15f;
    glm::vec3 hitbox_offset{ 0.0f, 0.085f, 0.0f };;

};
