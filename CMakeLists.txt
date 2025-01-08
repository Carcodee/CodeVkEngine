cmake_minimum_required(VERSION 3.26)
project(CodeVkRenderer VERSION 0.0.1)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(SOLUTION_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
message("Solution Dir is: ${SOLUTION_DIR}")

find_package(Vulkan)

message(STATUS "Found Vulkan in ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Found Vulkan libs in ${Vulkan_LIBRARIES}")


set(GLFW3_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/glfw/include")
set(GLFW3_LIBRARIES "${SOLUTION_DIR}/dependencies/glfw/lib-vc2022/glfw3.lib")
set(GLM_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/glm-master/glm")
set(STB_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/stb-master")
set(SPIRVCROSS_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/spirv-cross")
set(TINYGLTF_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/tinygltf-release")
set(IMGUI_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/imgui-docking")
set(PROFILER_EXT_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/LegitProfiler")
set(IMGUI_NODE_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/imgui-node-editor-master")


include_directories("${Vulkan_INCLUDE_DIRS}")
include_directories("${GLFW3_INCLUDE_DIR}")
include_directories("${GLM_INCLUDE_DIR}")
include_directories("${STB_INCLUDE_DIR}")
include_directories("${SPIRVCROSS_INCLUDE_DIR}")
include_directories("${TINYGLTF_INCLUDE_DIR}")
include_directories("${IMGUI_INCLUDE_DIR}")
include_directories("${PROFILER_EXT_INCLUDE_DIR}")
include_directories("${IMGUI_NODE_INCLUDE_DIR}")


file(GLOB_RECURSE dep_sources "${SPIRVCROSS_INCLUDE_DIR}/*.cpp" "${IMGUI_INCLUDE_DIR}/*.cpp" "${IMGUI_NODE_INCLUDE_DIR}/*.cpp")
file(GLOB_RECURSE engine_sources "${SOLUTION_DIR}/src/*.cpp")
file(GLOB_RECURSE engine_headers "${SOLUTION_DIR}/src/*.hpp")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(Engine "${SOLUTION_DIR}/src/main.cpp")

if(MSVC)
    target_compile_options(Engine PRIVATE /bigobj)
endif()

target_sources(Engine PRIVATE
        ${dep_sources}
        ${engine_headers}
        ${engine_sources})

target_link_libraries(Engine ${Vulkan_LIBRARIES} ${GLFW3_LIBRARIES})

set_target_properties(Engine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
