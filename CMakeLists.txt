cmake_minimum_required(VERSION 3.7)

project(Iris)
set(CMAKE_CXX_STANDARD 17)

find_package(Git)

if(GIT_FOUND)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
        message(STATUS "Initializing git repository...")
        execute_process(COMMAND ${GIT_EXECUTABLE} init WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} RESULT_VARIABLE GIT_INIT_RESULT)

        if(NOT GIT_INIT_RESULT EQUAL "0")
            message(FATAL_ERROR "Unable to initialize git repository.")
        endif()
    endif()

    message(STATUS "Retrieving git submodules...")

    set(SUBMODULES lib/vulkan;
            lib/glfw;
            lib/vk-bootstrap;
            lib/vma;
            lib/glm;
            lib/tinyobjloader
            lib/stb;
            lib/imgui)
    set(REPOSITORIES https://github.com/KhronosGroup/Vulkan-Headers.git;
            https://github.com/glfw/glfw.git;
            https://github.com/charles-lunarg/vk-bootstrap;
            https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git;
            https://github.com/g-truc/glm.git;
            https://github.com/tinyobjloader/tinyobjloader.git;
            https://github.com/nothings/stb.git;
            https://github.com/ocornut/imgui.git;)
    set(RELEASES main;
            3.2.1;
            main;
            master;
            master;
            release;
            master;
            master;)

    foreach(UPD_SUB IN LISTS SUBMODULES)
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote -- ${UPD_SUB} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} RESULT_VARIABLE GIT_SUBMOD_RESULT)

        list(FIND SUBMODULES ${UPD_SUB} SUB_INDEX)
        list(GET REPOSITORIES ${SUB_INDEX} SUB_URL)
        list(GET RELEASES ${SUB_INDEX} SUB_RELEASE)

        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule add ${SUB_URL} ${UPD_SUB} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${UPD_SUB} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} RESULT_VARIABLE GIT_SUBMOD_RESULT)

            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                message(FATAL_ERROR "Unable to retrieve submodule ${UPD_SUB}")
            endif()
        endif()

        execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${SUB_RELEASE} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/${UPD_SUB} RESULT_VARIABLE GIT_SUBMOD_RESULT)

        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(SEND_ERROR "Unable to checkout branch ${SUB_RELEASE} of repository ${UPD_SUB}")
        endif()
    endforeach()
else()
    message(FATAL_ERROR "Git not found.")
endif()

add_subdirectory(lib/glfw)
include_directories(lib/glfw/include)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/vulkan)
include_directories(lib/vulkan/include)
add_subdirectory(lib/vk-bootstrap)
include_directories(lib/vma/include)
include_directories(lib/glm)
add_subdirectory(lib/tinyobjloader)
include_directories(lib/tinyobjloader)
include_directories(lib/stb)
include_directories(lib/imgui lib/imgui/backends)

find_package(Vulkan REQUIRED)

file(GLOB IMGUI_FILES "./lib/imgui/*.h" "./lib/imgui/*.cpp" "./lib/imgui/backends/*glfw*" "./lib/imgui/backends/*vulkan*")

# shader compilation
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)
message(${PROJECT_SOURCE_DIR})
foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

# Compile and link all source files in the "src" directory
file(GLOB_RECURSE SRC_FILES "./scr/*.hpp" "./scr/*.cpp")
add_executable(Iris ${SRC_FILES} ${SPIRV_BINARY_FILES} ${IMGUI_FILES})

# Link against all libraries
target_link_libraries(Iris ${Vulkan_LIBRARIES} glfw vk-bootstrap::vk-bootstrap tinyobjloader)

# Set the working directory for the executable to the project directory
set_target_properties(Iris PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
target_compile_features(Iris PUBLIC cxx_std_17)