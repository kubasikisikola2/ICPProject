#include "GameClasses/Target.hpp"

bool Target::target_hit(const glm::vec3& rayOrigin,const glm::vec3& rayDir)
{
    const glm::vec3 sphereCenter = model->getPosition() + hitbox_offset;
    glm::vec3 oc = rayOrigin - sphereCenter;

    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f)
        return false;

    float sqrtD = std::sqrt(discriminant);

    float t1 = (-b - sqrtD) / (2.0f * a);
    float t2 = (-b + sqrtD) / (2.0f * a);

    if (t1 >= 0.0f) {
        return true;
    }
    if (t2 >= 0.0f) {
        return true;
    }

    return false;
}

void Target::set_model(std::shared_ptr<Model> model) {
    this->model = model;
}

void Target::set_hitbox_offset(glm::vec3 hitbox_offset) {
    this->hitbox_offset = hitbox_offset;
}

glm::vec3 Target::get_center() {
    return model->getPosition() + hitbox_offset;
}

std::shared_ptr<Model> Target::get_model() {
    return model;
}