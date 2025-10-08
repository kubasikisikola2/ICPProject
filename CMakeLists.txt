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


if(WIN32)
    #This I can't get to work on Windows; VS seems to require a properly set up solution/project and I can't get VSCode to work either
    find_package(OpenCV CONFIG REQUIRED PATHS "<path to your opencv>")

elseif(UNIX)
    find_package(OpenCV CONFIG REQUIRED)
    find_package(TBB REQUIRED) #Comment out if causes problems, Libor needs it on Linux
    target_link_libraries(ICPProject1 PRIVATE TBB::tbb)
endif()

add_executable(ICPProject1
    ICPProject1.cpp
    "src/App.cpp"
 "src/FpsMeter.cpp")

target_link_libraries(ICPProject1 PRIVATE
    fmt::fmt
    GLEW::GLEW
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
    ${OpenCV_LIBS}
)

target_include_directories(ICPProject1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_include_directories(ICPProject1
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)