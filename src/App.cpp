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

App::App()
{
    // default constructor
    // nothing to do here (so far...)
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
    //
    // Initialize pipeline: compile, link and use shaders
    //

    //SHADERS - define & compile & link
    const char* vertex_shader =
        "#version 460 core\n"
        "in vec3 attribute_Position;"
        "void main() {"
        "  gl_Position = vec4(attribute_Position, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 460 core\n"
        "uniform vec4 uniform_Color;"
        "out vec4 FragColor;"
        "void main() {"
        "  FragColor = uniform_Color;"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    shader_prog_ID = glCreateProgram();
    glAttachShader(shader_prog_ID, fs);
    glAttachShader(shader_prog_ID, vs);
    glLinkProgram(shader_prog_ID);

    //now we can delete shader parts (they can be reused, if you have more shaders)
    //the final shader program already linked and stored separately
    glDetachShader(shader_prog_ID, fs);
    glDetachShader(shader_prog_ID, vs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 
    // Create and load data into GPU using OpenGL DSA (Direct State Access)
    //

    // Create VAO + data description (similar to container)
    glCreateVertexArrays(1, &VAO_ID);

    GLint position_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Position");

    glEnableVertexArrayAttrib(VAO_ID, position_attrib_location);
    glVertexArrayAttribFormat(VAO_ID, position_attrib_location, sizeof(vertex::position) / sizeof(float), GL_FLOAT, GL_FALSE, offsetof(vertex, position));
    glVertexArrayAttribBinding(VAO_ID, position_attrib_location, 0); // (GLuint vaobj, GLuint attribindex, GLuint bindingindex)

    // Create and fill data
    glCreateBuffers(1, &VBO_ID);
    glNamedBufferData(VBO_ID, triangle_vertices.size() * sizeof(vertex), triangle_vertices.data(), GL_STATIC_DRAW);

    // Connect together
    glVertexArrayVertexBuffer(VAO_ID, 0, VBO_ID, 0, sizeof(vertex)); // (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
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

    // Activate shader program. There is only one program, so activation can be out of the loop. 
    // In more realistic scenarios, you will activate different shaders for different 3D objects.
    glUseProgram(shader_prog_ID);

    // Get uniform location in GPU program. This will not change, so it can be moved out of the game loop.
    GLint uniform_color_location = glGetUniformLocation(shader_prog_ID, "uniform_Color");
    if (uniform_color_location == -1) {
        std::cerr << "Uniform location is not found in active shader program. Did you forget to activate it?\n";
    }

    FpsMeter gl_fps_meter(std::chrono::milliseconds(FPS_METER_INTERVAL));
    double gl_fps{ 0.0 }; //unintentional surprised face LOL!

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

    double now = glfwGetTime();
    double begin_time = now;
    double last_time = now; // so that delta time is 0 at the beginning

    glClearColor(0, 0, 0, 0);

    float triangle_animation_speed = 120.0;
    float triangle_hue{};

    while (1)
    {
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
        double delta_time = begin_time - last_time;
        double time_step = game_paused ? 0 : game_speed * delta_time;

        triangle_hue += triangle_animation_speed * time_step;
        triangle_hue = std::fmod(triangle_hue, 360);
        hsv2rgb(triangle_hue, 1, 1, r, g, b);

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
        if (key == 27 || glfwWindowShouldClose(window)) {
            tracker_terminate = true;
            tracker_thread.join();
            break;
        }

        // clear canvas
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //DRAW CALLS HERE

        //set uniform parameter for shader
        // (try to change the color in key callback)          
        glUniform4f(uniform_color_location, r, g, b, a);

        //bind 3d object data
        glBindVertexArray(VAO_ID);

        // draw all VAO data
        glDrawArrays(GL_TRIANGLES, 0, triangle_vertices.size());

        if (show_imgui) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
            std::string title_string = "ICP Project [FPS: " + ss.str() + ", VSync: " + (is_vsync_on ? "ON" : "OFF") + "]";
            glfwSetWindowTitle(window, title_string.c_str());
        }

        // poll events, call callbacks, flip back<->front buffer
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

void App::destroy(void)
{
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