cmake_minimum_required(VERSION 3.28.0)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++ -lc++abi")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++ -lc++ -lc++abi")
add_compile_options(-std=c++20)
project(MyGame LANGUAGES CXX)
add_executable(MyGame main.cpp)
find_package(PkgConfig REQUIRED)
pkg_check_modules(VULKAN REQUIRED vulkan)
pkg_check_modules(SHADERC REQUIRED shaderc)
pkg_check_modules(ASSIMP REQUIRED assimp)
target_include_directories(MyGame
    PUBLIC
        $ENV{VULKAN_SDK}/Include
)
add_library(imgui_custom STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp    
)
target_include_directories(imgui_custom
    PUBLIC
        $ENV{VULKAN_SDK}/Include
)
target_include_directories(imgui_custom PUBLIC external/imgui)
find_package(odfaeg REQUIRED)
#find_package(assimp REQUIRED)
#find_package(Vulkan REQUIRED)
#find_package(shaderc REQUIRED)

target_link_libraries(MyGame
    PRIVATE
        odfaeg::odfaeg-core
        odfaeg::odfaeg-math
        odfaeg::odfaeg-window
        odfaeg::odfaeg-graphics
        odfaeg::odfaeg-entity
        odfaeg::odfaeg-physics
	    imgui_custom
)
