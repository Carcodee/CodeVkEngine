cmake_minimum_required(VERSION 3.26)
project(CodeVkRenderer VERSION 0.0.1 LANGUAGES CXX)

if(NOT MSVC)
    message(FATAL_ERROR
            "CodeVkRenderer requires the MSVC toolchain because its GLFW, Slang, "
            "and CodeCudaEngine dependencies are built as Visual C++ libraries. "
            "Select a Visual Studio toolchain instead of MinGW.")
endif()

# The bundled third-party libraries are release-runtime (/MD) binaries.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(SOLUTION_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
message("Solution Dir is: ${SOLUTION_DIR}")

find_package(Vulkan REQUIRED)

set(CODE_CUDA_ENGINE_ROOT "${SOLUTION_DIR}/dependencies/CodeCudaEngine" CACHE PATH
        "Installed CodeCudaEngine package prefix")
if(NOT EXISTS "${CODE_CUDA_ENGINE_ROOT}/lib/codeCudaLib.lib")
    message(FATAL_ERROR
            "Bundled CodeCudaEngine library is missing. Restore dependencies/CodeCudaEngine "
            "from a complete checkout of this repository.")
endif()
find_package(CodeCudaEngine CONFIG REQUIRED
        PATHS "${CODE_CUDA_ENGINE_ROOT}"
        NO_DEFAULT_PATH)

message(STATUS "Found Vulkan in ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Found Vulkan libs in ${Vulkan_LIBRARIES}")

get_filename_component(Vulkan_Base_Path ${Vulkan_INCLUDE_DIRS} DIRECTORY)
message(STATUS "Found Vulkan path in ${Vulkan_Base_Path}")

set(GLFW3_LIBRARIES "${SOLUTION_DIR}/dependencies/glfw/lib-vc2022/glfw3.lib")

set(GLFW3_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/glfw/include")
set(GLM_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/glm-master")
set(STB_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/stb-master")
set(SPIRVCROSS_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/spirv-cross")
set(TINYGLTF_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/tinygltf-release")
set(IMGUI_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/imgui-docking")
set(PROFILER_EXT_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/LegitProfiler")
set(IMGUI_NODE_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/imgui-node-editor-master")
set(SLANG_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/Slang/include")
set(SLANG_LIBRARIES "${SOLUTION_DIR}/dependencies/Slang/lib/slang-compiler.lib")
set(SLANG_RUNTIME_FILES
        "${SOLUTION_DIR}/dependencies/Slang/bin/slang-compiler.dll"
        "${SOLUTION_DIR}/dependencies/Slang/bin/slang-glslang.dll"
        "${SOLUTION_DIR}/dependencies/Slang/bin/slang-glsl-module.dll"
        "${SOLUTION_DIR}/dependencies/Slang/bin/slang-rt.dll")
set(NLOHMANN_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/nlohmann")
set(HAPPLY_INCLUDE_DIR "${SOLUTION_DIR}/dependencies/happly-master")

foreach(required_file IN ITEMS
        "${GLFW3_LIBRARIES}"
        "${SLANG_LIBRARIES}"
        ${SLANG_RUNTIME_FILES})
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR
                "Required binary dependency is missing: ${required_file}. "
                "Restore dependencies/ from a complete checkout of this repository.")
    endif()
endforeach()

        
file(GLOB_RECURSE dep_sources CONFIGURE_DEPENDS
        "${SPIRVCROSS_INCLUDE_DIR}/*.cpp"
        "${IMGUI_INCLUDE_DIR}/*.cpp"
        "${IMGUI_NODE_INCLUDE_DIR}/*.cpp")
file(GLOB_RECURSE engine_sources CONFIGURE_DEPENDS "${SOLUTION_DIR}/src/*.cpp")
file(GLOB_RECURSE engine_headers CONFIGURE_DEPENDS "${SOLUTION_DIR}/src/*.hpp")

set(CMAKE_VERBOSE_MAKEFILE ON)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_executable(Engine)
set_property(TARGET Engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${SOLUTION_DIR}")

if(MSVC)
    target_compile_options(Engine PRIVATE /bigobj)
endif()

if(MINGW)
    target_compile_options(Engine PRIVATE -Wa,-mbig-obj)
endif()

target_sources(Engine PRIVATE
        ${dep_sources}
        ${engine_headers}
        ${engine_sources})

target_include_directories(Engine PRIVATE
        "${GLFW3_INCLUDE_DIR}"
        "${GLM_INCLUDE_DIR}"
        "${STB_INCLUDE_DIR}"
        "${SPIRVCROSS_INCLUDE_DIR}"
        "${TINYGLTF_INCLUDE_DIR}"
        "${IMGUI_INCLUDE_DIR}"
        "${PROFILER_EXT_INCLUDE_DIR}"
        "${IMGUI_NODE_INCLUDE_DIR}"
        "${SLANG_INCLUDE_DIR}"
        "${NLOHMANN_INCLUDE_DIR}"
        "${HAPPLY_INCLUDE_DIR}")

target_link_libraries(Engine PRIVATE
        ${SLANG_LIBRARIES}
        Vulkan::Vulkan
        ${GLFW3_LIBRARIES}
        CodeCudaEngine::codeCudaLib
)

set_target_properties(Engine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


add_custom_command(
        TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_RUNTIME_FILES}
        "$<TARGET_FILE_DIR:Engine>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SOLUTION_DIR}/dependencies/Slang/bin/slang-standard-module-2026.13.1"
        "$<TARGET_FILE_DIR:Engine>/slang-standard-module-2026.13.1"
)

