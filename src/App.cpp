#include <iostream>
#include <fstream>
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
#include "AudioManager.hpp"
#include "MeshGen.hpp"

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
    
    glfwWindowHint(GLFW_SAMPLES, aa_sample_count);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(window_width, window_height, "OpenGL context", NULL, NULL);
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
    glfwSetWindowPosCallback(window, glfw_windowPositionCallback);

    glfwGetWindowPos(window, &window_pos_x, &window_pos_y);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void App::init_assets(void) {
    //initialize default texture
    Texture::init();

    std::filesystem::path filename = "../resources/models/cube_triangles_vnt.obj";
    std::vector<Vertex> v_wooden_box;
    std::vector<GLuint> i_wooden_box;
    if (!loadOBJ(filename, v_wooden_box, i_wooden_box)) {
        throw std::runtime_error("Loading failed: " + filename.string());

    }
    auto mesh_wooden_box = generate_cube_mesh();


    shader_library.emplace("text_shader", std::make_shared<ShaderProgram>(std::filesystem::path(TEXTURE_SHADER_PATH_VERT), std::filesystem::path(TEXTURE_SHADER_PATH_FRAG)));
    texture_library.emplace("wood_box_text", std::make_shared<Texture>("../resources/textures/box_rgb888.png"));
    mesh_library.emplace("wood_box_mesh", mesh_wooden_box);
    add_box_line(BOXES_COUNT, BOXES_START_X, BOXES_SPACING, BOXES_Y_POSITION, BOXES_Z_POSITION);

    std::vector<Vertex> vGun;
    std::vector<GLuint> iGun;
    filename = "../resources/models/gun.obj";
    if (!loadOBJ(filename, vGun, iGun)) {
        throw std::runtime_error("Loading failed: " + filename.string());

    }
    auto mesh_gun = std::make_shared<Mesh>(vGun, iGun, GL_TRIANGLES);

    texture_library.emplace("gun_text", std::make_shared<Texture>("../resources/textures/gun.jpg"));
    mesh_library.emplace("mesh_gun", mesh_gun);

    Model gunModel;
    gunModel.addMesh(mesh_library.at("mesh_gun"), shader_library.at("text_shader"), texture_library.at("gun_text"));
    scene.emplace("gun", std::make_shared<Model>(gunModel));

    init_room_assets();

    Model webcamModel;
    texture_library.emplace("webcam_text", std::make_shared<Texture>("../resources/textures/placeholder.jpg"));
    mesh_library.emplace("webcam_mesh", generate_webcam_mesh());
    webcamModel.addMesh(mesh_library.at("webcam_mesh"), shader_library.at("text_shader"), texture_library.at("webcam_text"));
    scene.emplace("webcam", std::make_shared<Model>(webcamModel));


    mesh_library.emplace("muzzle_flash_mesh", generate_webcam_mesh());
    texture_library.emplace("muzzle_flash_text", std::make_shared<Texture>("../resources/textures/muzzle_flash.png"));
    muzzle_flash_model = std::make_shared<Model>();
    muzzle_flash_model->addMesh(mesh_library.at("muzzle_flash_mesh"), shader_library.at("text_shader"), texture_library.at("muzzle_flash_text")
    );

    std::vector<Vertex> v_target;
    std::vector<GLuint> i_target;
    filename = "../resources/models/target.obj";
    if (!loadOBJ(filename, v_target, i_target)) {
        throw std::runtime_error("Loading failed: " + filename.string());

    }
    auto mesh_target = std::make_shared<Mesh>(v_target, i_target, GL_TRIANGLES);

    texture_library.emplace("target_text", std::make_shared<Texture>("../resources/textures/target.jpeg"));
    mesh_library.emplace("target_mesh", mesh_target);
   
    target_manager.init(N_TARGETS, mesh_library.at("target_mesh"), shader_library.at("text_shader"), texture_library.at("target_text"));

    std::cout << "succesfully initialized assets" << std::endl;
}

void App::init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    std::cout << "ImGUI version: " << ImGui::GetVersion() << "\n";
}

void App::load_music()
{
    std::vector<std::pair<std::string, std::string>> name_filename_pairs = {
        {"Doom", "../resources/music/03_E1M1_At_Doom_s_Gate.mp3"},
        {"ouch", "../resources/sfx/ouch.wav"},
        {"step1", "../resources/sfx/step1.wav"},
        {"step2", "../resources/sfx/step2.wav"}
    };

    bool success = true;
    for (auto name_filename : name_filename_pairs)
    {
        if (!AudioManager::getInstance().load(name_filename.first, name_filename.second))
        {
            success = false;
        }
    }
    
    if (!success)
    {
        throw std::runtime_error("Failed to load one or multiple sound files");
    }
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

        std::ifstream sett_file("../settings.json");
        nlohmann::json settings = nlohmann::json::parse(sett_file);

        if (settings["window_size"]["width"].is_number_integer()) {          
            window_width = settings["window_size"]["width"].template get<int>();
            if (window_width < 10)
            {
                window_width = 10;
            }
        }
        else {
            window_width = 800;
        }
        if (settings["window_size"]["height"].is_number_integer()) {
            window_height = settings["window_size"]["height"].template get<int>();
            if (window_height < 10)
            {
                window_height = 10;
            }
        }
        else {
            window_height = 600;
        }
        if (settings["aa_sample_count"].is_number_integer()) {
            aa_sample_count = settings["aa_sample_count"].template get<int>();
            if (aa_sample_count != 4 && aa_sample_count != 2 && aa_sample_count != 1 && aa_sample_count != 8)
            {
                aa_sample_count = 4;
            }
        }
        else {
            aa_sample_count = 4;
        }

        init_opencv();

        init_glfw();
        init_glew();

        check_gl_version();

        print_opencv_info();
        print_gl_info();
        print_glfw_info();
        print_glm_info();

        if (!AudioManager::getInstance().initMicrophone())
        {
            throw std::runtime_error("Microphone init failed");
        }

        glfwSwapInterval(is_vsync_on ? 1 : 0); // vsync

        init_assets();

        load_music();

        init_imgui();

        glfwShowWindow(window);
    }
    catch (std::exception const& e) {
        std::cerr << "App init failed : " << e.what() << std::endl;
        throw;
    }

    std::cout << "Initialized...\n";

    return true;
}

int App::run(void)
{

    FpsMeter gl_fps_meter(std::chrono::milliseconds(FPS_METER_INTERVAL));
    FpsMeter tracker_fps_meter(std::chrono::milliseconds(FPS_METER_INTERVAL));
    float tracker_fps = 0.0;
    float gl_fps = 0.0;


    cv::Mat face_frame;
    std::vector<cv::Point2f> face_pos;
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

    glfwGetFramebufferSize(window, &viewport_width, &viewport_height);
    glViewport(0, 0, viewport_width, viewport_height);
    update_projection_matrix();
    screenshot.create(viewport_height, viewport_width, CV_8UC3);

    AudioManager::getInstance().playBGM("Doom");

    player.set_floor_height(-1.0f);
    player.set_mode(PlayerMode::FirstPerson);
    gun.set_model(scene.at("gun"));
    gun.set_flash_model(muzzle_flash_model);

    while (!glfwWindowShouldClose(window))
    {
        // check if face is on webcam. if not --> pause
        if (!tracker_frame_deque.empty() && !tracker_pos_deque.empty())
        {
            face_frame = tracker_frame_deque.pop_front();
            face_pos = tracker_pos_deque.pop_front();
            if (face_pos.size() == 1) {
                paused_by_tracker = false;
            }
            else {
                paused_by_tracker = true;
            }

            tracker_fps_meter.update();

            if (tracker_fps_meter.is_updated()) {
                tracker_fps = tracker_fps_meter.get_fps();
            }

            texture_library.at("webcam_text")->replace_image(face_frame);
        }

        game_paused = paused_by_key || paused_by_tracker;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        draw_crosshair();
   
        // ImGui prepare render (only if required)
        if (show_imgui) {
            //ImGui::ShowDemoWindow(); // Enable mouse when using Demo!
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::SetNextWindowSize(ImVec2(250, 270)); 

            ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::Text("Total shots: %d", total_shots);
            ImGui::Text("Accuracy: %.1f%%", accuracy);
            ImGui::Text("FPS: %.1f", gl_fps);
            ImGui::Text("Tracker FPS: %.1f", tracker_fps);
            ImGui::Text("Microphone RMS: %.3f", AudioManager::getInstance().getMicLoudness());
            ImGui::Text("V-Sync: %s", is_vsync_on ? "ON" : "OFF");
            ImGui::Text("RMB - release mouse");
            ImGui::Text("TAB - show/hide GUI");
            ImGui::Text("P - pause");
            ImGui::Text("F11 - toggle fullscreen");
            ImGui::Text("M - toggle sound");
            ImGui::Text("V - toggle V-Sync");
            ImGui::Text("K - toggle camera");
            ImGui::Text("ESC - close app");
            ImGui::End();
        }

        //GAME STATE UPDATES HERE
        double delta_time = game_paused ? 0: (begin_time - last_time);

        // clear canvas
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // update player and gun logic and set camera
        player.update(window, delta_time, camera);
        update_projection_matrix();
        glm::mat4 view_matrix = camera.GetViewMatrix();
        gun.update(delta_time, camera.Position, camera.Front, camera.Up, camera.Right);

        //draw all models from scene
        for (auto &model : scene) {
            if (model.first == "webcam")
                continue;
            model.second->update(delta_time);
            model.second->draw(view_matrix, projection_matrix);
        }

        if (gun.is_muzzle_flash_active()){
            muzzle_flash_model->draw(view_matrix, projection_matrix);
        }

        
        target_manager.draw_targets(view_matrix, projection_matrix, now);
        draw_webcam();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        

        if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS)
        {
            take_screenshot();
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
            std::string title_string = std::string(WINDOW_TITLE) + " [" + (game_paused ? "Paused, " : "") + 
                "FPS: " + ss.str() + ", VSync: " + (is_vsync_on ? "ON" : "OFF") + "]";
            glfwSetWindowTitle(window, title_string.c_str());
        }

        // poll events, call callbacks, flip back<->front buffer
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

void App::shoot()
{
    glm::vec3 rayOrigin = camera.Position;
    glm::vec3 rayDir = glm::normalize(camera.Front);
    glm::vec3 taget_hit_position;
    total_shots++;

    if (target_manager.shot_fired(rayOrigin, rayDir, &taget_hit_position))
    {
        //play positional audio
        if (!muted) {
            AudioManager::getInstance().play2D("ouch");
        }
        total_hits++;
    }
    
    accuracy = ((float) total_hits / total_shots) * 100;
}

void App::draw_webcam() {
    auto webcam = scene.at("webcam");
    glDisable(GL_DEPTH_TEST);

    glm::mat4 hud_view = glm::mat4(1.0f);
    glm::mat4 hud_proj = glm::ortho(
        0.0f, static_cast<float>(viewport_width),
        0.0f, static_cast<float>(viewport_height),
        -1.0f, 1.0f
    );

    float webcamSize = static_cast<float>(computeWebcamSize());

    glm::mat4 webcam_model =
        glm::translate(glm::mat4(1.0f),
            glm::vec3(
                viewport_width - webcamSize,
                viewport_height - webcamSize,
                0.0f
            )) *
        glm::scale(glm::mat4(1.0f),
            glm::vec3(webcamSize, webcamSize, 1.0f));

    webcam->setModelMatrix(webcam_model);
    webcam->draw(hud_view, hud_proj);

    glEnable(GL_DEPTH_TEST);
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

int App::min_int(int x, int y)
{
    return x < y ? x : y;
}

int App::max_int(int x, int y)
{
    return x > y ? x : y;
}

//https://stackoverflow.com/a/31526753
GLFWmonitor* App::get_current_monitor(GLFWwindow* window)
{

	int monitor_count;
	int wx, wy, ww, wh;
	int mx, my, mw, mh;
	int overlap, best_overlap = 0;
	GLFWmonitor* best_monitor = NULL;
	GLFWmonitor** monitors;
	const GLFWvidmode* mode;

	glfwGetWindowPos(window, &wx, &wy);
	glfwGetWindowSize(window, &ww, &wh);
	monitors = glfwGetMonitors(&monitor_count);

	for (int i = 0; i < monitor_count; ++i)
	{
		mode = glfwGetVideoMode(monitors[i]);
		glfwGetMonitorPos(monitors[i], &mx, &my);
		mw = mode->width;
		mh = mode->height;

		overlap =
			max_int(0, min_int(wx + ww, mx + mw) - max_int(wx, mx)) *
			max_int(0, min_int(wy + wh, my + mh) - max_int(wy, my));

		if (best_overlap < overlap)
		{
			best_overlap = overlap;
			best_monitor = monitors[i];
		}
	}

	return best_monitor;

}

void App::take_screenshot(){
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

int App::computeWebcamSize()
{
    int base_size = static_cast<int>(std::min(viewport_width, viewport_height) * WEBCAM_SCALE_FACTOR);
    return base_size;
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


void App::draw_crosshair()
{
    float cx = viewport_width * 0.5f;
    float cy = viewport_height * 0.5f;
    float size = 10.0f;

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    ImU32 color = IM_COL32(255, 0, 0, 255);

    draw_list->AddLine(ImVec2(cx - size, cy), ImVec2(cx + size, cy), color, 2.0f);
    draw_list->AddLine(ImVec2(cx, cy - size), ImVec2(cx, cy + size), color, 2.0f);
}

void App::init_room_assets() {
    float size = ROOM_SIZE;
    float room_height = ROOM_HEIGHT;
    float half = size * 0.5f;

    float floor_uv_x = 10.0f;
    float floor_uv_y = 10.0f;

    float wall_uv_x = 10.0f;
    float wall_uv_y = 3.0f;

    float ceiling_uv_x = 10.0f;
    float ceiling_uv_y = 10.0f;

    texture_library.emplace("wall_text", std::make_shared<Texture>("../resources/textures/wall.png"));
    texture_library.emplace("ceiling_text", std::make_shared<Texture>("../resources/textures/floor.jpg"));
    texture_library.emplace("floor_text", std::make_shared<Texture>("../resources/textures/floor.jpg"));

    mesh_library.emplace("floor_mesh", generate_quad_mesh(
        { -half, 0.0f, -half },
        { half, 0.0f, -half },
        { half, 0.0f,  half },
        { -half, 0.0f,  half },
        { 0.0f, 1.0f, 0.0f },
        floor_uv_x, floor_uv_y
    ));
    Model floor_model;
    floor_model.addMesh(mesh_library.at("floor_mesh"), shader_library.at("text_shader"), texture_library.at("floor_text"));
    scene.emplace("floor", std::make_shared<Model>(floor_model));

    mesh_library.emplace("ceiling_mesh", generate_quad_mesh(
        { -half, room_height, -half },
        { -half, room_height,  half },
        { half, room_height,  half },
        { half, room_height, -half },
        { 0.0f, -1.0f, 0.0f },
        ceiling_uv_x, ceiling_uv_y
    ));
    Model ceiling_model;
    ceiling_model.addMesh(mesh_library.at("ceiling_mesh"), shader_library.at("text_shader"), texture_library.at("ceiling_text"));
    scene.emplace("ceiling", std::make_shared<Model>(ceiling_model));

    mesh_library.emplace("back_wall_mesh", generate_quad_mesh(
        { -half, 0.0f, -half },
        { half, 0.0f, -half },
        { half, room_height, -half },
        { -half, room_height, -half },
        { 0.0f, 0.0f, 1.0f },
        wall_uv_x, wall_uv_y
    ));
    Model back_wall_model;
    back_wall_model.addMesh(mesh_library.at("back_wall_mesh"), shader_library.at("text_shader"), texture_library.at("wall_text"));
    scene.emplace("back_wall", std::make_shared<Model>(back_wall_model));

    mesh_library.emplace("front_wall_mesh", generate_quad_mesh(
        { half, 0.0f, half },
        { -half, 0.0f, half },
        { -half, room_height, half },
        { half, room_height, half },
        { 0.0f, 0.0f, -1.0f },
        wall_uv_x, wall_uv_y
    ));
    Model front_wall_model;
    front_wall_model.addMesh(mesh_library.at("front_wall_mesh"), shader_library.at("text_shader"), texture_library.at("wall_text"));
    scene.emplace("front_wall", std::make_shared<Model>(front_wall_model));

    mesh_library.emplace("left_wall_mesh", generate_quad_mesh(
        { -half, 0.0f,  half },
        { -half, 0.0f, -half },
        { -half, room_height, -half },
        { -half, room_height,  half },
        { 1.0f, 0.0f, 0.0f },
        wall_uv_x, wall_uv_y
    ));
    Model left_wall_model;
    left_wall_model.addMesh(mesh_library.at("left_wall_mesh"), shader_library.at("text_shader"), texture_library.at("wall_text"));
    scene.emplace("left_wall", std::make_shared<Model>(left_wall_model));

    mesh_library.emplace("right_wall_mesh", generate_quad_mesh(
        { half, 0.0f, -half },
        { half, 0.0f,  half },
        { half, room_height,  half },
        { half, room_height, -half },
        { -1.0f, 0.0f, 0.0f },
        wall_uv_x, wall_uv_y
    ));
    Model right_wall_model;
    right_wall_model.addMesh(mesh_library.at("right_wall_mesh"), shader_library.at("text_shader"), texture_library.at("wall_text"));
    scene.emplace("right_wall", std::make_shared<Model>(right_wall_model));
}

void App::add_box_line(int count, float startX, float spacing, float y, float z) {
    Model cubeTemplate;
    cubeTemplate.addMesh(
        mesh_library.at("wood_box_mesh"),
        shader_library.at("text_shader"),
        texture_library.at("wood_box_text")
    );

    for (int i = 0; i < count; ++i) {
        auto box = std::make_shared<Model>(cubeTemplate);

        glm::mat4 modelMatrix = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(startX + i * spacing, y, z)
        );

        box->setModelMatrix(modelMatrix);

        scene.emplace("box_" + std::to_string(i), box);
    }
}