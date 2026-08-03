cmake_minimum_required(VERSION 3.28.0)
option(ODFAEG_USE_MODULES "Use C++20 modules instead of includes" ON)
if(ODFAEG_USE_MODULES)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
else()
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(ODFAEG VERSION 2.0.0 LANGUAGES CXX)
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Simulate ID: ${CMAKE_CXX_SIMULATE_ID}")
message(STATUS "Frontend: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Compiler: Clang")

    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        message(STATUS "→ Clang-cl (MSVC STL)")
        set(ODFAEG_COMPILER "clang-msvc")
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        message(STATUS "→ Clang MinGW/MSYS2 (libc++)")
        set(ODFAEG_COMPILER "clang-msys2")
    else()
        message(STATUS "→ Clang Linux (libc++ or libstdc++)")
        set(ODFAEG_COMPILER "clang-linux")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Compiler: GCC MinGW")
    set(ODFAEG_COMPILER "gcc-mingw")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Compiler: MSVC")
    set(ODFAEG_COMPILER "msvc")

endif()
if (ODFAEG_COMPILER STREQUAL "clang-msys2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++ -lc++abi")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++ -lc++ -lc++abi")
endif()
if (ODFAEG_COMPILER STREQUAL "clang-msys2")
set(GLSLANG_VALIDATOR "C:/VulkanSDK/1.4.350.0/Bin/glslangValidator.exe")
endif()

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -gno-column-info")

# ============================================================
# 1) Base target (exporte les includes ODFAEG + vcpkg)
# ============================================================
add_library(odfaeg-base INTERFACE
	include/odfaeg/config.hpp       
        include/odfaeg/Window/windowHandle.hpp
        )
target_include_directories(odfaeg-base
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<BUILD_INTERFACE:${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/>	
        $<INSTALL_INTERFACE:include>
)

# ============================================================
# 2) Build all ODFAEG modules
# ============================================================
add_subdirectory(src/odfaeg)

# ============================================================
# 4) Install everything in ONE export (like SFML)
# ============================================================
install(
    TARGETS
        odfaeg-base        
    EXPORT odfaeg-targets 
)

# Install ODFAEG headers
install(    
    DIRECTORY include/
    DESTINATION include
)

# Install shaders
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/Shader/
    DESTINATION Shader
)
# ============================================================
# 5) Install the global ODFAEG package config
# ============================================================

include(CMakePackageConfigHelpers)
if (ODFAEG_USE_MODULES)
configure_package_config_file(
    cmake/odfaegModulesConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/odfaegModulesConfig.cmake
    INSTALL_DESTINATION lib/cmake/odfaeg
)
install(
    EXPORT odfaeg-targets
    FILE odfaegModulesTargets.cmake
    NAMESPACE odfaeg::
    DESTINATION lib/cmake/odfaeg
)
install(
    FILES odfaegConfig.cmake
    DESTINATION lib/cmake/odfaeg
)
else()
configure_package_config_file(
    cmake/odfaegIncludesConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/odfaegIncludesConfig.cmake
    INSTALL_DESTINATION lib/cmake/odfaeg
)
install(
    EXPORT odfaeg-targets
    FILE odfaegIncludesConfig.cmake
    NAMESPACE odfaeg::
    DESTINATION lib/cmake/odfaeg
)
install(
    EXPORT odfaeg-targets
    FILE odfaegIncludesTargets.cmake
    NAMESPACE odfaeg::
    DESTINATION lib/cmake/odfaeg
)
install(
    FILES odfaegConfig.cmake
    DESTINATION lib/cmake/odfaeg
)
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/external/
    DESTINATION include/odfaeg/external
)
endif()
