#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "FpsMeter.hpp"
#include "Config.hpp"
#include "TrackerThread.hpp"
//#include "SyncedDequePartialImpl.hpp"
#include "Assets.hpp"
#include "ShaderProgram.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "GameClasses/Player.hpp"
#include "GameClasses/Gun.hpp"
#include "GameClasses/Target.hpp"
#include "GameClasses/TargetManager.hpp"
#include "GameClasses/SmallBox.hpp"

class App {
public:
    App();
    bool firstMouse;
    bool init(void);
    void destroy(void);
    int run(void);
    int computeWebcamSize();
    ~App();
private:

    GLFWwindow* window = nullptr;
    bool is_vsync_on{ true };
    bool show_imgui{ true };
    float game_speed{ 1.0 };
    bool paused_by_key{ false };
    bool muted{ false }, music_muted{ false };
    bool game_paused{ true };

    GLuint shader_prog_ID{ 0 };
    GLuint VBO_ID{ 0 };
    GLuint VAO_ID{ 0 };

    std::vector<Vertex> triangle_vertices =
    {
        {{0.0f,  0.5f,  0.0f}},
        {{0.5f, -0.5f,  0.0f}},
        {{-0.5f, -0.5f,  0.0f}}
    };

    cv::VideoCapture capture;
    std::atomic<bool> tracker_terminate; //if true terminate the tracker loop
    std::atomic<bool> tracker_buffer_empty;
    std::vector<cv::Point2f> tracker_result;
    std::mutex points_result_mutex;
    std::thread tracker_thread;

    //this is just for image display in the main thread
    //pos deque to make the crosshair synchronized with the image
    synced_deque<std::vector<cv::Point2f>> tracker_pos_deque;
    synced_deque<cv::Mat> tracker_frame_deque;

    // hash map for storing shader programs
    std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shader_library;

    //hash map for storing meshes
    std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_library;

    // hash map for textures
    std::unordered_map<std::string, std::shared_ptr<Texture>> texture_library;

    // all objects on the scene
    std::unordered_map<std::string, std::shared_ptr<Model>> scene;

    int viewport_width, viewport_height;
    float FOV_degrees = 60.0f;
    glm::mat4 projection_matrix = glm::identity<glm::mat4>();
    Camera camera;
    Player player;
    Gun gun;
    TargetManager target_manager;
    SmallBox smol_box;
    
    std::shared_ptr<Model> muzzle_flash_model;

    cv::Mat screenshot;

    uint32_t total_shots = 0;
    uint32_t total_hits = 0;
    float accuracy = 0.0f;

    int window_pos_x, window_pos_y, window_width, window_height, aa_sample_count;
    bool fullscreen = false;
    double cursorLastX{ 0 };
    double cursorLastY{ 0 };

    void init_glew();
    void init_glfw();
    void init_opencv();
    void init_assets();
    void init_imgui();
    void load_music();

    void check_gl_version();

    void print_opencv_info();
    void print_glfw_info();
    void print_gl_info();
    void print_glm_info();

    void draw_webcam();
    void take_screenshot();

    //callbacks
    static void glfw_windowPositionCallback(GLFWwindow* window, int xpos, int ypos);
    static void glfw_cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfw_error_callback(int error, const char* description);
    static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

    void update_projection_matrix(void);

    static GLFWmonitor* get_current_monitor(GLFWwindow* window);

    static int min_int(int x, int y);
    static int max_int(int x, int y);
    void draw_crosshair();
    void shoot();
    void init_room_assets();
    void add_box_line(int count, float startX, float spacing, float y, float z);
};

