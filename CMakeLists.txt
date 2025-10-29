cmake_minimum_required(VERSION 3.27)
project(turned
    VERSION 0.0.0.1
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 17)
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DPROJECT_NAME_VERSION="${PROJECT_NAME} {PROJECT_VERSION}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)

include_directories(src/ third_party/)
link_directories(third_party/)
add_executable(main src/main.cpp)

target_link_libraries(main OpenGL::GL
    glfw
    glm::glm
    spdlog::spdlog
)
