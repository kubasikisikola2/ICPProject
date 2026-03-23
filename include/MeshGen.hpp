std::shared_ptr<Mesh> generate_cube_mesh();
std::shared_ptr<Mesh> generate_quad_mesh(
    const glm::vec3& p0,
    const glm::vec3& p1,
    const glm::vec3& p2,
    const glm::vec3& p3,
    const glm::vec3& normal,
    float uvScaleX,
    float uvScaleY);
std::shared_ptr<Mesh> generate_webcam_mesh();