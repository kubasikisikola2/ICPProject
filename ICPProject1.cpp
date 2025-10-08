#include <iostream>
#include <numeric>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <opencv2/opencv.hpp>

#include "App.hpp"

App app;

cv::Point2f centroid_nonzero(cv::Mat& scene, cv::Scalar& lower_threshold, cv::Scalar& upper_threshold)
{
	cv::Mat scene_hsv;
	cv::cvtColor(scene, scene_hsv, cv::COLOR_BGR2HSV);

	cv::Mat scene_threshold;
	cv::inRange(scene_hsv, lower_threshold, upper_threshold, scene_threshold);

	cv::namedWindow("scene_threshold", 0);
	cv::imshow("scene_threshold", scene_threshold);

	std::vector<cv::Point> whitePixels;
	cv::findNonZero(scene_threshold, whitePixels);
	int whiteCnt = whitePixels.size();

	cv::Point whiteAccum = std::accumulate(whitePixels.begin(), whitePixels.end(), cv::Point(0.0, 0.0));

	cv::Point2f centroid_normalized(0.0f, 0.0f);
	if (whiteCnt > 0)
	{
		cv::Point centroid = { whiteAccum.x / whiteCnt, whiteAccum.y / whiteCnt };
		centroid_normalized = { static_cast<float>(centroid.x) / scene.cols, static_cast<float>(centroid.y) / scene.rows };
	}

	return centroid_normalized;
}

int hsv_finder(const char *filepath)
{
    // load image
    //TODO: figure out how to find the directory regardless of current working directory
	cv::Mat scene = cv::imread(filepath); //can be JPG,PNG,GIF,TIFF,...

	if (scene.empty())
	{
		std::cerr << "not found" << std::endl;
		exit(EXIT_FAILURE);
	}


	cv::namedWindow("scene", 0);
	cv::imshow("scene", scene);

	int hm = 0, sm = 0, vm = 0, hx = 179, sx = 255, vx = 255;
	cv::createTrackbar("HMin", "scene", &hm, 179);
	cv::createTrackbar("SMin", "scene", &sm, 255);
	cv::createTrackbar("VMin", "scene", &vm, 255);
	cv::createTrackbar("HMax", "scene", &hx, 179);
	cv::createTrackbar("SMax", "scene", &sx, 255);
	cv::createTrackbar("VMax", "scene", &vx, 255);

	cv::Scalar lower_threshold;
	cv::Scalar upper_threshold;
	do {
		// Get current positions of trackbars
		// HSV ranges between (0-179, 0-255, 0-255).
		lower_threshold = cv::Scalar(
			cv::getTrackbarPos("HMin", "scene"),
			cv::getTrackbarPos("SMin", "scene"),
			cv::getTrackbarPos("VMin", "scene"));

		upper_threshold = cv::Scalar(
			cv::getTrackbarPos("HMax", "scene"),
			cv::getTrackbarPos("SMax", "scene"),
			cv::getTrackbarPos("VMax", "scene"));

		// compute centroid 
		auto start = std::chrono::system_clock::now();
		auto centroid = centroid_nonzero(scene, lower_threshold, upper_threshold);
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << "elapsed time: " << elapsed_seconds.count() << " sec" << std::endl;
		std::cout << "found normalized: " << centroid << std::endl;

		//display result
		cv::Mat scene_cross;
		scene.copyTo(scene_cross);
		app.draw_cross_normalized(scene_cross, centroid, 30);
		cv::imshow("scene", scene_cross);

		
	} while (cv::waitKey(1) != 27); //message loop with 1ms delay untill ESC

    return(EXIT_SUCCESS); //TODO: getting a segmentation fault on exit
}

int main()
{
    //Commented out code is stored for later as a reference or for moving to classes or local libraries
    
    try {
        if (app.init())
            return app.run();
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
    

    /*
    std::cout << "Hello!\n";

// ===  important for 2D raster graphics  ===

    // demo: use OpenCV
    cv::Mat x(cv::Size(100,100), CV_8UC3);
    cv::namedWindow("test_window");

    // try to open first camera of any type and grab single image
    auto cam = cv::VideoCapture(0, cv::CAP_ANY);
    if (cam.isOpened())
        cam.read(x);

    cv::imshow("test_window", x);
    cv::pollKey();

    
// ===  important for 3D graphics  ===
    
    // demo: use GLM
    glm::vec3 vec{};
    std::cout << glm::to_string(vec) << '\n';

    // demo: use glfw
    if (!glfwInit()) exit(100);
    GLFWwindow* w = glfwCreateWindow(800, 600, "test", nullptr, nullptr);
    if (!w) exit(111);
    glfwMakeContextCurrent(w);

    // demo: use glew
    auto s = glewInit();
    if (s != GLEW_OK) exit(123);

    // wait for ESC key in GLFW window
    while (!glfwWindowShouldClose(w)) {
        if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(w, GLFW_TRUE);
        }
        
        glfwPollEvents();
    }

    std::cout << "Bye!\n";
    return 0;
    */

    /*
    const char *map_path = "../resources/HSV-MAP.png";
    const char *lightbulb_path = "../resources/lightbulb.jpg";

    return hsv_finder(lightbulb_path);
    */
}