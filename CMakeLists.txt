cmake_minimum_required(VERSION 3.5)  # Updated to remove deprecation warning
project(TextbookOpenGL)

set(CMAKE_CXX_STANDARD 11)

# Set the path to vcpkg toolchain file - Assuming this is correct as you've set it
set(CMAKE_TOOLCHAIN_FILE "C:/Users/jackp/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Find packages
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)  # Ensure GLEW is found

add_executable(TextbookOpenGL src/main.cpp)

# Link libraries
target_link_libraries(TextbookOpenGL PRIVATE glad::glad glfw GLEW::GLEW)
