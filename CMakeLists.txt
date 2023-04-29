cmake_minimum_required(VERSION 3.15)
project(AudioVisualizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
set(EXTERNALS ${PROJECT_SOURCE_DIR}/external)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${EXTERNALS}/glad/include)

# Collect all .h and .cpp files in the specified directories
file(GLOB_RECURSE HEADER_FILES "include/*.h")
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

set(GLAD_SOURCES ${EXTERNALS}/glad/src/glad.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${GLAD_SOURCES})
target_link_libraries(${PROJECT_NAME}
    ole32
    oleaut32
    uuid
    ksuser
    mfplat
    mfuuid
    wmcodecdspuuid
    mfreadwrite
    mf
    mmdevapi
    glfw
    ${OPENGL_gl_LIBRARY}
)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} -mwindows)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE include)