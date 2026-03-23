#include "TrackerThread.hpp"

void tracker_thread_func(cv::VideoCapture& capture,
    std::atomic<bool>& tracker_terminate,
    std::atomic<bool>& tracker_buffer_empty,
    synced_deque<cv::Mat>& frames_deque,
    synced_deque<std::vector<cv::Point2f>>& points_deque) {

    cv::CascadeClassifier face_cascade;
    face_cascade = cv::CascadeClassifier("../resources/haarcascade_frontalface_default.xml");
    cv::Mat image_intruder = cv::imread("../resources/intruder.jpg");
    cv::Mat image_no_face = cv::imread("../resources/no_face.jpg");

    cv::Mat frame;
    cv::Mat resized_frame;
    cv::Mat output_frame;
    std::vector<cv::Point2f> faces;
    std::vector<cv::Point2f> last_faces;
    uint32_t frame_counter = FRAMES_PER_DETECTION;
    while (!tracker_terminate)
    {

        if (!capture.read(frame))
        {
            tracker_buffer_empty = true;
            break;
        }

        cv::resize(frame, resized_frame, cv::Size(WEBCAM_CAPTURE_SIZE, WEBCAM_CAPTURE_SIZE));
        frame_counter++;

        if (frame_counter % FRAMES_PER_DETECTION == 0) {
            last_faces = find_face(resized_frame, face_cascade);
        }

        faces = last_faces;

        if(faces.size() == 1){
            output_frame = resized_frame.clone();
            draw_cross_normalized(output_frame, faces[0], 15);
        }
        else if (faces.size() == 0) {
            cv::resize(image_no_face, output_frame, resized_frame.size());
        }
        else {
            cv::resize(image_intruder, output_frame, resized_frame.size());
        }

        cv::flip(output_frame, output_frame, 0);
        points_deque.push_back(faces);
        frames_deque.push_back(output_frame.clone());

        points_deque.notify();
        frames_deque.notify();
    }
}

std::vector<cv::Point2f> find_face(cv::Mat& frame, cv::CascadeClassifier& face_cascade)
{
    cv::Point2f center(0.0f, 0.0f);

    cv::Mat scene_detect;
    cv::cvtColor(frame, scene_detect, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(scene_detect, scene_detect);
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

void draw_cross_normalized(cv::Mat& img, cv::Point2f center_normalized, int size)
{
    center_normalized.x = std::clamp(center_normalized.x, 0.0f, 1.0f);
    center_normalized.y = std::clamp(center_normalized.y, 0.0f, 1.0f);
    size = std::clamp(size, 1, std::min(img.cols, img.rows));

    cv::Point2f center_absolute(center_normalized.x * img.cols, center_normalized.y * img.rows);

    cv::Point2f p1(center_absolute.x - (float)size / 2, center_absolute.y);
    cv::Point2f p2(center_absolute.x + (float)size / 2, center_absolute.y);
    cv::Point2f p3(center_absolute.x, center_absolute.y - (float)size / 2);
    cv::Point2f p4(center_absolute.x, center_absolute.y + (float)size / 2);

    cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
    cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}