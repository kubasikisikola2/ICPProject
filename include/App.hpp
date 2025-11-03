#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

#include "FpsMeter.hpp"
#include "Config.hpp"
#include "TrackerThread.hpp"
//#include "SyncedDequePartialImpl.hpp"
#include "Assets.hpp"

class App {
public:
    App();

    bool init(void);
    void init_assets();

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

    void print_opencv_info();
    void print_gl_info();
    void print_glm_info();

    GLFWwindow* window = nullptr;

    GLuint shader_prog_ID{ 0 };
    GLuint VBO_ID{ 0 };
    GLuint VAO_ID{ 0 };

    std::vector<vertex> triangle_vertices =
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
};

