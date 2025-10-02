#pragma once

#include <opencv2/opencv.hpp>

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
    cv::VideoCapture capture;
};

