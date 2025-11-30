#include <iostream>
#include <numeric>
#include <execution>

#include <opencv2/core/types.hpp>
#include <nlohmann/json.hpp>

// OpenGL headers
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

// ImGUI headers
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "App.hpp"
#include "ObjectLoader.hpp"

App::App()
{
    // default constructor
    // nothing to do here (so far...)
    firstMouse = true;
    std::cout << "Constructed...\n";
}

void App::init_opencv()
{
    capture = cv::VideoCapture(0, cv::CAP_ANY);
    if (!capture.isOpened())
        throw std::runtime_error("Can not open camera!");
    else
    {
        std::cout << "Camera opened successfully.\n";
    }
}

void App::init_glfw()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        throw std::runtime_error("GLFW init failed!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(800, 600, "OpenGL context", NULL, NULL);
    if (window == nullptr) {
        throw std::runtime_error("Window creation failed!");
    }

    glfwSetWindowUserPointer(window, this);

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetCursorPosCallback(window, glfw_cursorPositionCallback);
}

void App::init_glew()
{
    GLenum glew_retval = glewInit();
    if (glew_retval != GLEW_OK)
        throw std::runtime_error(std::string("GLEW init failed!") + (const char*)glewGetErrorString(glew_retval));
    else
        std::cout << "GLEW version: " << glewGetString(GLEW_VERSION) << '\n';

    GLenum wglew_retval = wglewInit();
    if (wglew_retval != GLEW_OK)
        throw std::runtime_error(std::string("WGLEW init failed!") + (const char*)glewGetErrorString(wglew_retval));
    else
        std::cout << "WGLEW initialized" << '\n';

    if (!GLEW_ARB_direct_state_access)
    {
        throw std::runtime_error("No DSA :-( *sad whine*");
    }

    if (GLEW_ARB_debug_output)
    {
        glDebugMessageCallback(MessageCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

        //default is asynchronous debug output, use this to simulate glGetError() functionality
        //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        std::cout << "GL_DEBUG enabled." << std::endl;
    }
    else
        std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;

    if (GLEW_ARB_multisample)
    {
        std::cout << "GL antialiasing is supported." << std::endl;
    }
    else
    {
        std::cout << "GL antialiasing is NOT supported." << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
}

void App::check_gl_version()
{
    GLint gl_version_major, gl_version_minor;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_version_major);
    glGetIntegerv(GL_MINOR_VERSION, &gl_version_minor);

    if (gl_version_major != 4 || gl_version_minor != 6)
    {
        throw std::runtime_error("We're not using requested OpenGL version!");
    }
}

void App::print_opencv_info()
{
    std::cout << "Capture capabilities:"
        << " width = " << capture.get(cv::CAP_PROP_FRAME_WIDTH)
        << ", height = " << capture.get(cv::CAP_PROP_FRAME_HEIGHT)
        << '\n';
}

void App::print_glfw_info()
{
    std::cout << "GLFW version: " << glfwGetVersionString() << '\n';
}

void App::print_glm_info()
{
    std::cout << "GLM version: " << GLM_VERSION_MAJOR << '.' << GLM_VERSION_MINOR << '.'
        << GLM_VERSION_PATCH << "rev" << GLM_VERSION_REVISION << '\n';
}

void App::print_gl_info()
{
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    std::cout << "GPU vendor is: " << (vendor == nullptr ? "Unknown" : vendor) << '\n';

    const char* renderer = (const char*)glGetString(GL_RENDERER);
    std::cout << "GL renderer is: " << (renderer == nullptr ? "Unknown" : renderer) << '\n';

    const char* gl_version = (const char*)glGetString(GL_VERSION);
    std::cout << "GL version is: " << (gl_version == nullptr ? "Unknown" : gl_version) << '\n';

    const char* glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::cout << "GLSL version is: " << (glsl_version == nullptr ? "Unknown" : glsl_version) << '\n';

    GLint gl_context_profile_mask;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &gl_context_profile_mask);

    if (gl_context_profile_mask & GL_CONTEXT_CORE_PROFILE_BIT) {
        std::cout << "We are using CORE profile\n";
    }
    else {
        if (gl_context_profile_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
            std::cout << "We are using COMPATIBILITY profile\n";
            throw std::runtime_error("We're not using CORE profile!");
        }
        else {
            throw std::runtime_error("What??");
        }
    }

    GLint gl_context_flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &gl_context_flags);

    if (gl_context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) {
        std::cout << "This GL context is forward compatible\n";
    }
    if (gl_context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        std::cout << "This GL context is a debug context\n";
    }
    if (gl_context_flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
    {
        std::cout << "This GL context supports robust memory access\n";
    }
    if (gl_context_flags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
    {
        std::cout << "This GL context doesn't report errors\n";
    }
}

void App::init_assets(void) {

    // load shaders from file to shader_library 
    shader_library.emplace("simple_shader", std::make_shared<ShaderProgram>(std::filesystem::path("../resources/shaders/basic.vert"), std::filesystem::path("../resources/shaders/basic.frag")));
 
    // Load mesh
    std::filesystem::path filename = "../resources/models/triangle.obj";
    if (!std::filesystem::exists(filename)) {
        throw std::runtime_error("File does not exist: " + filename.string());
    }

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    if (!loadOBJ(filename, vertices, indices)) {
        throw std::runtime_error("Loading failed: " + filename.string());
    }

    auto mesh = std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);

    auto mesh_ptr = std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);
    mesh_library.emplace("simple_mesh", mesh_ptr);

    // Load model and attach mesh
    Model my_model;
    my_model.addMesh(mesh_ptr, shader_library.at("simple_shader"));
    scene.emplace("simple_object", my_model);

    std::vector<Vertex> vertices2;
    std::vector<GLuint> indices2;
    filename = "../resources/models/bunny_tri_vnt.obj";
    if (!loadOBJ(filename, vertices2, indices2)) {
        throw std::runtime_error("Loading failed: " + filename.string());
    }
   
    auto mesh2 = std::make_shared<Mesh>(vertices2, indices2, GL_TRIANGLES);
    mesh_library.emplace("bunny_mesh", mesh_ptr);

    Model model2;
    my_model.addMesh(mesh2, shader_library.at("simple_shader"));
    scene.emplace("bunny", my_model);

}

void App::init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    std::cout << "ImGUI version: " << ImGui::GetVersion() << "\n";
}

bool App::init()
{
    try {
        std::cout << "Current working directory: " << std::filesystem::current_path().generic_string() << '\n';

        if (!std::filesystem::exists("../resources"))
        {
            throw std::runtime_error("Directory 'resources' not found. Various media files are expected to be there.");
        }

        if (!std::filesystem::exists("../screenshots"))
        {
            std::filesystem::create_directory("../screenshots");
        }

        init_opencv();

        init_glfw();
        init_glew();

        check_gl_version();

        print_opencv_info();
        print_gl_info();
        print_glfw_info();
        print_glm_info();

        glfwSwapInterval(is_vsync_on ? 1 : 0); // vsync

        init_assets();

        init_imgui();

        glfwShowWindow(window);
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
    GLfloat r, g, b, a;
    r = g = b = a = 1.0f; //white color

    glm::vec4 my_rgba(r, g, b, a);

    FpsMeter gl_fps_meter(std::chrono::milliseconds(FPS_METER_INTERVAL));
    double gl_fps{ 0.0 }; //unintentional surprised face LOL!

    std::string fps_string;

    cv::Mat face_frame;
    std::vector<cv::Point2f> face_pos;

    int baseline = 0;
    cv::Size fps_text_size = cv::getTextSize(fps_string, cv::FONT_HERSHEY_SIMPLEX, FPS_TEXT_FONT_SCALE, FPS_TEXT_LINE_WIDTH, &baseline);
    cv::Point fps_text_pos(10, fps_text_size.height + 10);
    cv::Scalar fps_text_color(0, 255, 0);
    cv::Mat show_frame;

    tracker_thread = std::thread(tracker_thread_func,
                               std::ref(capture), 
                               std::ref(tracker_terminate),
                               std::ref(tracker_buffer_empty),
                               std::ref(tracker_frame_deque),
                               std::ref(tracker_pos_deque));

    double now = glfwGetTime();
    double begin_time = now;
    double last_time = now; // so that delta time is 0 at the beginning

    bool paused_by_tracker = false;

    glClearColor(0, 0, 0, 0);

    float triangle_animation_speed = 120.0;
    float triangle_hue{};

    glfwGetFramebufferSize(window, &viewport_width, &viewport_height);
    glViewport(0, 0, viewport_width, viewport_height);
    update_projection_matrix();
    screenshot.create(viewport_height, viewport_width, CV_8UC3);

    //set initial camera position
    //camera.Position = glm::vec3(0, 0, 10);

    while (!glfwWindowShouldClose(window))
    {
        // Find face
        if (tracker_buffer_empty) {
            std::cout << "Couldn't get new frame";
            break;
        }

        if (!tracker_frame_deque.empty() && !tracker_pos_deque.empty())
        {
            face_frame = tracker_frame_deque.pop_front();
            face_pos = tracker_pos_deque.pop_front();
            if (face_pos.size() > 0) {
                draw_cross_normalized(face_frame, face_pos[0], 15);
            }

            // show frame only when one person is watching
            if (face_pos.size() == 1) {
                show_frame = face_frame;
                paused_by_tracker = false;
            }
            else if (face_pos.size() == 0) {
                cv::resize(image_no_face, show_frame, cv::Size(face_frame.cols, face_frame.rows));
                paused_by_tracker = true;
            }
            else if (face_pos.size() > 1) {
                cv::resize(image_intruder, show_frame, cv::Size(face_frame.cols, face_frame.rows));
                paused_by_tracker = true;
            }

            fps_meter.update();

            if (fps_meter.is_updated()) {
                double fps = fps_meter.get_fps();
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << fps;
                fps_string = "FPS: " + ss.str();
                //std::cout << fps_string << std::endl;
            }

            cv::putText(show_frame, fps_string, fps_text_pos, FPS_TEXT_FONT, FPS_TEXT_FONT_SCALE, fps_text_color, FPS_TEXT_LINE_WIDTH);
            cv::imshow(WINDOW_TITLE, show_frame);
        }

        // ImGui prepare render (only if required)
        if (show_imgui) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            //ImGui::ShowDemoWindow(); // Enable mouse when using Demo!
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::SetNextWindowSize(ImVec2(250, 100));

            ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::Text("V-Sync: %s", is_vsync_on ? "ON" : "OFF");
            ImGui::Text("FPS: %.1f", gl_fps);
            ImGui::Text("(press RMB to release mouse)");
            ImGui::Text("(hit D to show/hide info)");
            ImGui::End();
        }

        //GAME STATE UPDATES HERE
        bool game_paused = paused_by_key || paused_by_tracker;
        double delta_time = begin_time - last_time;
        double time_step = game_paused ? 0 : game_speed * delta_time;

        triangle_hue += triangle_animation_speed * time_step;
        triangle_hue = std::fmod(triangle_hue, 360);
        hsv2rgb(triangle_hue, 1, 1, r, g, b);

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            scene.at("bunny").rotate(glm::vec3(0.0f, 180.0f * time_step, 0.0f));
        }

        // clear canvas
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        auto& current_shader = shader_library.at("simple_shader");
        current_shader->use();
        current_shader->setUniform("my_color", my_rgba);
        //set View matrix = set CAMERA
        camera.ProcessInput(window, delta_time);
        update_projection_matrix();
        glm::mat4 view_matrix = camera.GetViewMatrix();
        current_shader->setUniform("uV_m", view_matrix);
        current_shader->setUniform("uP_m", projection_matrix);

        //draw all models from scene
        for (auto &model : scene) {
            model.second.update(now);
            model.second.draw();
        }

        if (show_imgui) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS)
        {
            glReadPixels(0, 0, screenshot.cols, screenshot.rows, GL_BGR, GL_UNSIGNED_BYTE, screenshot.data);
            cv::flip(screenshot, screenshot, 0);
            auto screenshot_now = std::chrono::system_clock::now();
            auto screenshot_time_t = std::chrono::system_clock::to_time_t(screenshot_now);
            std::stringstream filename;
            filename << "../screenshots/" + std::string(SCREENSHOT_FILE_NAME) + '_';
            filename << std::put_time(std::localtime(&screenshot_time_t), SCREENSHOT_TIMESTAMP_FORMAT);
            filename << ".jpg";
            cv::imwrite(filename.str().c_str(), screenshot);
        }

        glfwSwapBuffers(window);

        now = glfwGetTime();
        last_time = begin_time;
        begin_time = now;

        gl_fps_meter.update();
        if (gl_fps_meter.is_updated())
        {
            gl_fps = gl_fps_meter.get_fps();
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << gl_fps;
            std::string title_string = std::string(WINDOW_TITLE) + " [FPS: " + ss.str() + ", VSync: " + (is_vsync_on ? "ON" : "OFF") + "]";
            glfwSetWindowTitle(window, title_string.c_str());
        }

        // poll events, call callbacks, flip back<->front buffer
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

void App::destroy(void)
{
    // Terminate tracker
    if (tracker_thread.joinable())
    {
        tracker_terminate = true;
        tracker_thread.join();
    }
    // clean up ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // OpenGL clean-up
    if (shader_prog_ID)
        glDeleteProgram(shader_prog_ID);
    if (VBO_ID)
        glDeleteBuffers(1, &VBO_ID);
    if (VAO_ID)
        glDeleteVertexArrays(1, &VAO_ID);

    // clean-up OpenCV
    cv::destroyAllWindows();
    // release camera
    if (capture.isOpened())
    {
        capture.release();
    }
}

App::~App()
{
    destroy();
    std::cout << "Bye...\n";
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

// https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB_alternative
void App::hsv2rgb(float h, float s, float v, float& r, float& g, float& b)
{
    auto f = [&](uint n) {
        auto k = [=]() { return fmod(n + h / 60, 6); };
        return v - v * s * std::fmax(0, std::min(k(), std::min(4 - k(), 1.0)));
        };
    r = f(5); g = f(3); b = f(1);
}

void App::update_projection_matrix(void)
{
    if (viewport_height <= 0)
    {
        viewport_height = 1;
    }
    float ratio = static_cast<float>(viewport_width) / viewport_height;
    glViewport(0, 0, viewport_width, viewport_height);
    projection_matrix = glm::perspective(glm::radians(FOV_degrees), ratio, NEAR_CLIP_PLANE, FAR_CLIP_PLANE);
}