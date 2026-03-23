#include "GameClasses/TargetManager.hpp"

TargetManager::TargetManager()
    : rng(std::random_device{}()),
    dist_x(min_x, max_x),
    dist_y(min_y, max_y)
{
}



glm::vec3 TargetManager::generate_position()
{
    return glm::vec3(dist_x(rng), dist_y(rng), plane_z);
}

void TargetManager::init(int count, std::shared_ptr<Mesh> mesh, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Texture> texture)
{
    target_count = count;

    current_targets.clear();

    current_targets.reserve(target_count);

    for (int i = 0; i < target_count; ++i)
    {
        auto model = std::make_shared<Model>();
        model->addMesh(mesh, shader, texture);
        model->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
        model->setPosition(generate_position());

        Target target;
        target.set_model(model);
        current_targets.push_back(target);
    }
}

void TargetManager::respawn_target(size_t index)
{
    if (index >= current_targets.size())
        return;

    current_targets[index].get_model()->setPosition(generate_position());
}



bool TargetManager::shot_fired(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3* target_hit_pos)
{
    for (size_t i = 0; i < current_targets.size(); ++i)
    {
        if (current_targets[i].target_hit(rayOrigin, rayDir))
        {
            if (target_hit_pos != nullptr)
            {
                *target_hit_pos = current_targets[i].get_center(); 
            }
            respawn_target(i);
            return true;
        }
    }

    return false;
}

void TargetManager::draw_targets(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, double dt){
    for (auto& model : current_targets)
    {
        model.get_model()->update(dt);
        model.get_model()->draw(view_matrix, projection_matrix);
    }
}