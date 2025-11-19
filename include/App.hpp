#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include "FpsMeter.hpp"
#include "Config.hpp"
#include "TrackerThread.hpp"
//#include "SyncedDequePartialImpl.hpp"
#include "Assets.hpp"
#include "ShaderProgram.hpp"
#include "Mesh.hpp"
#include "Model.hpp"

class App {
public:
    App();

    bool init(void);
    void destroy(void);

    int run(void);

    void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size);
    void draw_cross(cv::Mat& img, int x, int y, int size);
    cv::Point2f find_object_luma(cv::Mat & frame);
    cv::Point2f find_object_chroma(cv::Mat & frame);

    ~App();
private:
    void init_glew();
    void init_glfw();
    void init_opencv();
    void init_assets();
    void init_imgui();

    void check_gl_version();

    void print_opencv_info();
    void print_glfw_info();
    void print_gl_info();
    void print_glm_info();

    //callbacks
    static void glfw_error_callback(int error, const char* description);
    static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

    void hsv2rgb(float h, float s, float v, float& r, float& g, float& b);

    GLFWwindow* window = nullptr;
    bool is_vsync_on{ true };
    bool show_imgui{ true };
    float game_speed{ 1.0 };
    bool game_paused{ false };

    GLuint shader_prog_ID{ 0 };
    GLuint VBO_ID{ 0 };
    GLuint VAO_ID{ 0 };

    std::vector<Vertex> triangle_vertices =
    {
        {{0.0f,  0.5f,  0.0f}},
        {{0.5f, -0.5f,  0.0f}},
        {{-0.5f, -0.5f,  0.0f}}
    };

    FpsMeter fps_meter{ std::chrono::milliseconds(FPS_METER_INTERVAL)};

    cv::VideoCapture capture;
    cv::Mat image_intruder;
    cv::Mat image_no_face;
    std::atomic<bool> tracker_terminate; //if true terminate the tracker loop
    std::atomic<bool> tracker_buffer_empty;
    std::vector<cv::Point2f> tracker_result;
    std::mutex points_result_mutex;

    //this is just for image display in the main thread
    //pos deque to make the crosshair synchronized with the image
    synced_deque<std::vector<cv::Point2f>> tracker_pos_deque;
    synced_deque<cv::Mat> tracker_frame_deque;

    // hash map for storing shader programs
    std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shader_library;

    //hash map for storing meshes
    std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_library;

    // all objects on the scene
    std::unordered_map<std::string, Model> scene;
};

