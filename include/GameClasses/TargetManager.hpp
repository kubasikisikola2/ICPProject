#pragma once
#include "Target.hpp"
#include <random>

class TargetManager {
private:
	std::vector<Target> current_targets;

	int target_count = 5;
	float plane_z = -7.0f;

	float min_x = -4.0f;
	float max_x = 4.0f;
	float min_y = 0.5f;
	float max_y = 3.5f;
	std::mt19937 rng;
	std::uniform_real_distribution<float> dist_x;
	std::uniform_real_distribution<float> dist_y;
	
	void respawn_target(size_t index);

	glm::vec3 generate_position();
public:
	TargetManager();
	bool shot_fired(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3* target_hit_pos);
	void draw_targets(const glm::mat4& view_matrix, const glm::mat4& projection_matrix, double dt);
	void init(int count, std::shared_ptr<Mesh> mesh, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Texture> texture);
};