cmake_minimum_required(VERSION 3.28.0)
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
project(ODFAEGEditor LANGUAGES CXX)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
#set(CMAKE_PREFIX_PATH "$ENV{HOME}/odfaeg-install")

#set(CMAKE_TOOLCHAIN_FILE "/home/laurent/vcpkg/scripts/buildsystems/vcpkg.cmake")
#Packages conan
find_package(odfaeg REQUIRED)
find_package(assimp REQUIRED)
find_package(shaderc REQUIRED)
find_package(imgui REQUIRED)
find_package(Vulkan REQUIRED)
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp    
)
target_include_directories(imgui PUBLIC external/imgui
${glfw3_INCLUDE_DIRS})
add_executable(odfaeg-editor main.cpp)
target_link_libraries(odfaeg-editor
    PRIVATE
        odfaeg::odfaeg-core
        odfaeg::odfaeg-math
        odfaeg::odfaeg-window
        odfaeg::odfaeg-graphics
        odfaeg::odfaeg-entity
        odfaeg::odfaeg-physics
)

target_link_libraries(odfaeg-editor
    PRIVATE 
        imgui
    	Vulkan::Vulkan)
target_include_directories(odfaeg-editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
