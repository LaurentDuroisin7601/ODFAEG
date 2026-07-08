cmake_minimum_required(VERSION 3.28.0)
#set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)
#set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++ -lc++abi")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++ -lc++ -lc++abi")
add_compile_options(-std=c++20)
project(MyGame LANGUAGES CXX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(VULKAN REQUIRED vulkan)
pkg_check_modules(SHADERC REQUIRED shaderc)


find_package(odfaeg REQUIRED)
find_package(assimp REQUIRED)
#find_package(Vulkan REQUIRED)
#find_package(shaderc REQUIRED)
add_executable(MyGame main.cpp)
target_include_directories(MyGame
    PUBLIC
        $ENV{VULKAN_SDK}/Include
)
target_link_libraries(MyGame
    PRIVATE
        odfaeg::odfaeg-core
        odfaeg::odfaeg-math
        odfaeg::odfaeg-window
        odfaeg::odfaeg-graphics
        odfaeg::odfaeg-entity
        odfaeg::odfaeg-physics
)
