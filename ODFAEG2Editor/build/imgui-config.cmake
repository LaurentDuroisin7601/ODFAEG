########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(imgui_FIND_QUIETLY)
    set(imgui_MESSAGE_MODE VERBOSE)
else()
    set(imgui_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/imguiTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${imgui_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(imgui_VERSION_STRING "1.91.0-docking")
set(imgui_INCLUDE_DIRS ${imgui_INCLUDE_DIRS_RELEASE} )
set(imgui_INCLUDE_DIR ${imgui_INCLUDE_DIRS_RELEASE} )
set(imgui_LIBRARIES ${imgui_LIBRARIES_RELEASE} )
set(imgui_DEFINITIONS ${imgui_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${imgui_BUILD_MODULES_PATHS_RELEASE} )
    message(${imgui_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


