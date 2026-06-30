cmake_minimum_required(VERSION 3.28.0)
# Conan toolchain
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
#set(CMAKE_TOOLCHAIN_FILE "/home/laurent/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(ODFAEG VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS "/usr/local/bin/clang-scan-deps")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)


#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -gno-column-info")

# ============================================================
# 1) Base target (exporte les includes ODFAEG + vcpkg)
# ============================================================
add_library(odfaeg-base INTERFACE
	include/odfaeg/config.hpp
        include/odfaeg/Graphics/windowHandle.hpp
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
install(
    EXPORT odfaeg-targets
    FILE odfaegTargets.cmake
    NAMESPACE odfaeg::
    DESTINATION lib/cmake/odfaeg
)
include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/odfaegConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/odfaegConfig.cmake
    INSTALL_DESTINATION lib/cmake/odfaeg
)
install(
    FILES odfaegConfig.cmake
    DESTINATION lib/cmake/odfaeg
)
