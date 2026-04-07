#include <memory>
#include "Model.hpp"

class SmallBox {
public:
    void update(float dt, const glm::vec3& cam_pos);
    void knock();
    void set_model(std::shared_ptr<Model> model);
    void set_knock_name(const std::string snd_name);
    void set_position(const glm::vec3& position);

private:
    std::shared_ptr<Model> model;

    std::string knock_snd_name;
    float target_y_rot = 0.0f, current_y_rot = 0.0f;

    const float rot_speed = 5.0f;
};