cmake_minimum_required(VERSION 3.10)

project(ICPProject1)
set(OpenCV_DIR "C:/opencv/build/x64/vc16")

find_package(OpenCV REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)


add_executable(ICPProject1 ICPProject1.cpp)

target_link_libraries(ICPProject1 PRIVATE fmt::fmt)
target_link_libraries(ICPProject1 PRIVATE GLEW::glew)
target_link_libraries(ICPProject1 PRIVATE glfw)
target_link_libraries(ICPProject1 PRIVATE glm::glm)
target_link_libraries(ICPProject1 PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(ICPProject1 PRIVATE ${OpenCV_LIBS})
target_include_directories(ICPProject1 PRIVATE ${OpenCV_INCLUDE_DIRS})


