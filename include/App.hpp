#pragma once

#include <opencv2/opencv.hpp>
#include "FpsMeter.hpp"
#include "Config.hpp"
#include "SyncedDequePartialImpl.hpp"

class App {
public:
    App();

    bool init(void);
    int run(void);

    void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size);
    void draw_cross(cv::Mat& img, int x, int y, int size);
    cv::Point2f find_object_luma(cv::Mat & frame);
    cv::Point2f find_object_chroma(cv::Mat & frame);

    ~App();
private:
    FpsMeter fps_meter{ std::chrono::milliseconds(FPS_METER_INTERVAL)};

    cv::VideoCapture capture;
    cv::CascadeClassifier face_cascade;
    std::vector<cv::Point2f> find_face(cv::Mat& frame);

    std::atomic<bool> tracker_terminate; //if true terminate the tracker loop
    std::atomic<bool> tracker_buffer_empty;
    std::atomic<std::pair<float, float>> tracker_latest_pos;

    //this is just for image display in the main thread
    //pos deque to make the crosshair synchronized with the image
    synced_deque<std::pair<float, float>> tracker_pos_deque;
    synced_deque<cv::Mat> tracker_frame_deque;
};

