#pragma once

#include <opencv2/opencv.hpp>
#include "FpsMeter.hpp"
#include "Config.hpp"

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

};

