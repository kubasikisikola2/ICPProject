#include "TrackerThread.hpp"

void tracker_thread_func(cv::VideoCapture& capture,
    std::atomic<bool>& tracker_terminate,
    std::atomic<bool>& tracker_buffer_empty,
    synced_deque<cv::Mat>& frames_deque,
    synced_deque<std::vector<cv::Point2f>>& points_deque) {

    cv::CascadeClassifier face_cascade;
    face_cascade = cv::CascadeClassifier("../resources/haarcascade_frontalface_default.xml");


    cv::Mat frame;
    while (!tracker_terminate)
    {

        if (!capture.read(frame))
        {
            tracker_buffer_empty = true;
            break;
        }

        std::vector<cv::Point2f> faces = find_face(frame, face_cascade);

        points_deque.push_back(faces);
        frames_deque.push_back(frame.clone());

        points_deque.notify();
        frames_deque.notify();
    }
}

std::vector<cv::Point2f> find_face(cv::Mat& frame, cv::CascadeClassifier& face_cascade)
{
    cv::Point2f center(0.0f, 0.0f);

    cv::Mat scene_detect;
    cv::cvtColor(frame, scene_detect, cv::COLOR_BGR2GRAY);
    cv::resize(scene_detect, scene_detect, cv::Size(), DETECT_SIZE_SCALE_FACTOR, DETECT_SIZE_SCALE_FACTOR);

    std::vector<cv::Rect> faces;
    std::vector<cv::Point2f> center_points_norm;

    face_cascade.detectMultiScale(scene_detect, faces, DETECT_SCALE_FACTOR, DETECT_MIN_NEIGHBORS, 0, cv::Size(MIN_FACE_SIZE, MIN_FACE_SIZE));

    for (int i = 0; i < faces.size(); i++) {
        // calculating normalized coordinates of the face
        center.x = (faces[i].x + faces[i].width / 2.0) / scene_detect.cols;
        center.y = (faces[i].y + faces[i].height / 2.0) / scene_detect.rows;
        center_points_norm.push_back(center);
    }

    return center_points_norm;
}