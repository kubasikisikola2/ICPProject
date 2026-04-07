#include "GameClasses/SmallBox.hpp"

#include "AudioManager.hpp"
#include "Config.hpp"

void SmallBox::update(float dt, const glm::vec3& cam_pos)
{
	if (AudioManager::getInstance().getMicLoudness() > BOX_RMS_THRESH) {
		glm::vec3 position = model->getPosition();
		target_y_rot = glm::degrees(glm::atan(cam_pos.x - position.x, cam_pos.z - position.z));
	}

	/*float rot_diff = target_y_rot - current_y_rot;
	if (glm::abs(rot_diff) > 0.001f) {
		float rot_dir = (rot_diff > 0) ? 1.0f : -1.0f;
		float rot_step = rot_dir * rot_speed * dt;

		if (glm::abs(rot_step) >= glm::abs(rot_diff)) {
			current_y_rot = target_y_rot;
		}
		else {
			current_y_rot += rot_step;
		}
	}*/
	current_y_rot = glm::mix(current_y_rot, target_y_rot, rot_speed * dt);

	model->setEulerAngles(glm::vec3(0.0f, current_y_rot, 0.0f));
}

void SmallBox::knock()
{
	glm::vec3 position = model->getPosition();
	AudioManager::getInstance().play3D(knock_snd_name, position.x, position.y, position.z, BOX_KNOCK_VOLUME);
}

void SmallBox::set_model(std::shared_ptr<Model> model)
{
	this->model = model;
}

void SmallBox::set_knock_name(const std::string snd_name)
{
	knock_snd_name = snd_name;
}

void SmallBox::set_position(const glm::vec3& position)
{
	model->setPosition(position);
}
