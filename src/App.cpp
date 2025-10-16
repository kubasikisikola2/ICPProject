#include <iostream>
#include <numeric>
#include <execution>
#include <opencv2/core/types.hpp>

#include "App.hpp"

App::App()
{
    // default constructor
    // nothing to do here (so far...)
    std::cout << "Constructed...\n";
}

bool App::init()
{
    try {
        //open first available camera, using any API available (autodetect) 
        capture = cv::VideoCapture(0, cv::CAP_ANY);
    
        //open video file
        //capture = cv::VideoCapture("video.mkv");

        if (!capture.isOpened())
        { 
            throw("Couldn't open capture!");
        }
        else
        {
            std::cout << "Source: " << 
                ": width=" << capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
                ", height=" << capture.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
        }
    }
    catch (std::exception const& e) {
        std::cerr << "App init failed : " << e.what() << std::endl;
        throw;
    }

    image_intruder = cv::imread("../resources/intruder.jpg");;
    image_no_face = cv::imread("../resources/no_face.jpg");


    std::cout << "Initialized...\n";

    return true;
}

int App::run(void)
{
    
    cv::Mat frame, scene;
    cv::Point2f center;
    std::string fps_string;

    int baseline = 0;
    cv::Size fps_text_size = cv::getTextSize(fps_string, cv::FONT_HERSHEY_SIMPLEX, FPS_TEXT_FONT_SCALE, FPS_TEXT_LINE_WIDTH, &baseline);
    cv::Point fps_text_pos(10, fps_text_size.height + 10);
    cv::Scalar fps_text_color(0, 255, 0);
    cv::Mat show_frame;

    std::thread tracker_thread(tracker_thread_func,
                               std::ref(capture), 
                               std::ref(tracker_terminate),
                               std::ref(tracker_buffer_empty),
                               std::ref(tracker_frame_deque),
                               std::ref(tracker_pos_deque));
   
    while (1)
    {

        if (tracker_buffer_empty) {
            std::cout << "Couldn't get new frame";
            break;
        }
        tracker_frame_deque.wait();
        cv::Mat frame = tracker_frame_deque.pop_front();

        tracker_pos_deque.wait();
        std::vector<cv::Point2f> face_pos = tracker_pos_deque.pop_front();
        
        if (face_pos.size() > 0) {
            draw_cross_normalized(frame, face_pos[0], 15);
        }
        // show frame only when one person is watching
        if (face_pos.size() == 1) {
            show_frame = frame;
        }
        else if (face_pos.size() == 0) {
            cv::resize(image_no_face, show_frame, cv::Size(frame.cols, frame.rows));
        }
        else if (face_pos.size() > 1) {
            cv::resize(image_intruder, show_frame, cv::Size(frame.cols, frame.rows));
        }

        fps_meter.update();

        if (fps_meter.is_updated()) {
            double fps = fps_meter.get_fps();
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << fps;
            fps_string = "FPS: " + ss.str();
            std::cout << fps_string << std::endl;
        }

        cv::putText(frame, fps_string, fps_text_pos, FPS_TEXT_FONT, FPS_TEXT_FONT_SCALE, fps_text_color, FPS_TEXT_LINE_WIDTH);
        cv::imshow(WINDOW_TITLE, show_frame);
        int key = cv::waitKey(1);
        if (key == 27) {
            tracker_terminate = true;
            tracker_thread.join();
            break;
        }
    }
    return EXIT_SUCCESS;
}

App::~App()
{
    // clean-up
    cv::destroyAllWindows();
    std::cout << "Bye...\n";

    if (capture.isOpened())
    {
        capture.release();
    }
}

void App::draw_cross(cv::Mat& img, int x, int y, int size)
{
    cv::Point p1(x - size / 2, y);
    cv::Point p2(x + size / 2, y);
    cv::Point p3(x, y - size / 2);
    cv::Point p4(x, y + size / 2);

    cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
    cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}

void App::draw_cross_normalized(cv::Mat& img, cv::Point2f center_normalized, int size)
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

cv::Point2f App::find_object_luma(cv::Mat & frame)
{
    //Copy the frame
    cv::Mat loc_frame;
    frame.copyTo(loc_frame);

    // convert to grayscale, create threshold, sum white pixels
    // compute centroid of white pixels (average X,Y coordinate of all white pixels)
    cv::Point2f center;
    cv::Point2f center_normalized;
    int tot = 0;

    for (int y = 0; y < frame.rows; y++) //y
    {
        for (int x = 0; x < frame.cols; x++) //x
        {
            // load source pixel
            cv::Vec3b pixel = frame.at<cv::Vec3b>(y, x);

            // compute temp grayscale value (convert from colors to Y)
            unsigned char Y = 0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0];

            // FIND THRESHOLD (value 0..255)
            if (Y < 240) {
                // set output pixel black
                loc_frame.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
            }
            else {
                // set output pixel white
                loc_frame.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);

                ++tot;
                center.x += (x - center.x) / tot;
                center.y += (y - center.y) / tot;
            }
        }
    }

    center_normalized.x = center.x / frame.cols;
    center_normalized.y = center.y / frame.rows;
    return center_normalized;
}

cv::Point2f App::find_object_chroma(cv::Mat & frame)
{
    double h_low = 150.0;
    double s_low = 125.0;
    double v_low = 130.0;
    double h_hi = 255.0;
    double s_hi = 255.0;
    double v_hi = 255.0;

    cv::Mat scene_hsv, scene_threshold;

    cv::cvtColor(frame, scene_hsv, cv::COLOR_BGR2HSV);

    cv::Scalar lower_threshold = cv::Scalar(h_low, s_low, v_low);
    cv::Scalar upper_threshold = cv::Scalar(h_hi, s_hi, v_hi);
    cv::inRange(scene_hsv, lower_threshold, upper_threshold, scene_threshold);

    // Perform a morphological operation
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10, 10));
	//cv::morphologyEx(scene_threshold, scene_threshold, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 2);

    // Find all non-zero (ie. white) pixels in thresholded image
    std::vector<cv::Point> whitePixels;
    cv::findNonZero(scene_threshold, whitePixels);

    // Count white pixels
    int whiteCnt = whitePixels.size();

    // Count SUM of X_coords, Y_coords of white pixels
    // You need at least C++17 for std::reduce()
    //cv::Point2f whiteAccum = std::reduce(whitePixels.begin(), whitePixels.end());

    // or faster = parallel version, with automatic multi-threading
    cv::Point2f whiteAccum = std::reduce(std::execution::par_unseq, whitePixels.begin(), whitePixels.end());

    // Divide by whiteCnt to get average, ie. centroid (only if whiteCnt != 0 !!!)
    cv::Point2f centroid_absolute = whiteAccum / whiteCnt;
    // Compute NORMALIZED coordinates
    cv::Point2f centroid_normalized = { centroid_absolute.x / frame.cols, centroid_absolute.y / frame.rows };

    return centroid_normalized;
}


