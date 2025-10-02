#include <iostream>
#include <opencv2/opencv.hpp>


int main() {
    cv::Mat mat = cv::Mat::eye(3, 3, CV_8UC1);
    std::cout << "3x3 Identity Matrix:\n" << mat << std::endl;
    return 0;
}
