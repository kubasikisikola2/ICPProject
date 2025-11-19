cmake_minimum_required(VERSION 3.10)

project(ICPProject1 VERSION 2025.09.30 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

#set(OpenCV_DIR "C:/opencv/build/x64/vc16")
#find_package(OpenCV REQUIRED)

find_package(fmt CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

#Libor is gonna try this in Linux
#set(WITH_TBB OFF CACHE BOOL "Enable/disable TBB backend")
#set(BUILD_TBB OFF CACHE BOOL "Build TBB" FORCE)
#set(HAVE_TBB OFF CACHE BOOL "Have TBB" FORCE)

if(WIN32)
    set(OpenCV_DIR "C:/opencv/build/x64/vc16/lib")
    find_package(OpenCV CONFIG REQUIRED)
elseif(UNIX)
    find_package(OpenCV CONFIG REQUIRED)
    find_package(TBB REQUIRED) #Libor needs it in Linux
endif()

add_executable(ICPProject1
    ICPProject1.cpp
    "src/App.cpp"
    "src/FpsMeter.cpp"
    "src/TrackerThread.cpp"
    "src/Callbacks.cpp"
 "src/ShaderProgram.cpp" "src/ObjectLoader.cpp")

target_link_libraries(ICPProject1 PRIVATE
    fmt::fmt
    GLEW::GLEW
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
    imgui::imgui
    ${OpenCV_LIBS}
)

if(UNIX)
    target_link_libraries(ICPProject1 PRIVATE TBB::tbb)
endif()

target_include_directories(ICPProject1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_include_directories(ICPProject1
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)