#pragma once

#include "SyncedDequePartialImpl.hpp"
#include <opencv2/opencv.hpp>
#include "Config.hpp"



void tracker_thread_func(cv::VideoCapture& capture,
    std::atomic<bool>& tracker_terminate,
    std::atomic<bool>& tracker_buffer_empty,
    synced_deque<cv::Mat>& frames_deque,
    synced_deque<std::vector<cv::Point2f>>& points_deque);

std::vector<cv::Point2f> find_face(cv::Mat& frame, cv::CascadeClassifier& face_cascade);

