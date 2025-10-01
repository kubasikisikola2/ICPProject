#pragma once

class App {
public:
    App();

    bool init(void);
    int run(void);

    void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size);
    void draw_cross(cv::Mat& img, int x, int y, int size);

    ~App();
private:
};

