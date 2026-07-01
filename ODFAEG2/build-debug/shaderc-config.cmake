########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(shaderc_FIND_QUIETLY)
    set(shaderc_MESSAGE_MODE VERBOSE)
else()
    set(shaderc_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/shadercTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${shaderc_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(shaderc_VERSION_STRING "2025.3")
set(shaderc_INCLUDE_DIRS ${shaderc_INCLUDE_DIRS_DEBUG} )
set(shaderc_INCLUDE_DIR ${shaderc_INCLUDE_DIRS_DEBUG} )
set(shaderc_LIBRARIES ${shaderc_LIBRARIES_DEBUG} )
set(shaderc_DEFINITIONS ${shaderc_DEFINITIONS_DEBUG} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${shaderc_BUILD_MODULES_PATHS_DEBUG} )
    message(${shaderc_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


