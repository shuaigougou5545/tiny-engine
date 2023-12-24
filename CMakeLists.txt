cmake_minimum_required(VERSION 3.5.0)
project(tiny-engine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

# 获取当前系统的处理器架构
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(SYSTEM_ARCHITECTURE "arm64")
    message("System Processor: arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(SYSTEM_ARCHITECTURE "x86_64")
endif()

set (THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

file(GLOB IMGUI_FILES "third_party/imgui/*.cpp") # clang

# add source files => .cpp
set(SOURCE_FILES 
    src/main.cpp
    src/model_object.h
    src/model_object.cpp
    src/shader.h
    src/debug.h
    ${THIRD_PARTY_DIR}/glad/src/glad.c # glad
    ${IMGUI_FILES} # imgui
)

# 添加外部库的头文件
include_directories(
    ${THIRD_PARTY_DIR}/glad/include
    ${THIRD_PARTY_DIR}/glfw/include
    ${THIRD_PARTY_DIR}/glm/include
    ${THIRD_PARTY_DIR}/stb_image/include
    ${THIRD_PARTY_DIR}/imgui 
    ${THIRD_PARTY_DIR}/tinyobjloader/include
)

add_executable(tiny-engine ${SOURCE_FILES})

# 链接系统库
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    target_link_libraries(tiny-engine ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
endif()


# 链接外部库
target_link_libraries(tiny-engine
    ${THIRD_PARTY_DIR}/glfw/lib-${SYSTEM_ARCHITECTURE}/libglfw.3.dylib # 链接动态库
)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
