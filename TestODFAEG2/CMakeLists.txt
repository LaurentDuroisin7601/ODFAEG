cmake_minimum_required(VERSION 4.3)
project(MyGame LANGUAGES CXX)
set(CMAKE_C_COMPILER clang-18)
set(CMAKE_CXX_COMPILER clang++-18)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
#set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)

#set(CLANG_SCAN_DEPS_PATH /usr/local/bin)
#set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS
#        clang-scan-deps
#)
#set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_PREFIX_PATH "$ENV{HOME}/odfaeg-install")
#list(APPEND CMAKE_PREFIX_PATH "/home/laurent/vcpkg/installed/x64-linux")
add_compile_options(-I/usr/lib/llvm-18/include)
#add_compile_options(-stdlib=libc++)
find_package(odfaeg REQUIRED)
find_package(assimp REQUIRED)
add_executable(MyGame main.cpp)

target_link_libraries(MyGame
    PRIVATE
        odfaeg::odfaeg-core
        odfaeg::odfaeg-math
        odfaeg::odfaeg-window
        odfaeg::odfaeg-graphics
        odfaeg::odfaeg-entity
        odfaeg::odfaeg-physics
)
