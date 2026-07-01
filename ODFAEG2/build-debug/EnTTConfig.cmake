########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(EnTT_FIND_QUIETLY)
    set(EnTT_MESSAGE_MODE VERBOSE)
else()
    set(EnTT_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/EnTTTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${entt_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(EnTT_VERSION_STRING "3.13.0")
set(EnTT_INCLUDE_DIRS ${entt_INCLUDE_DIRS_DEBUG} )
set(EnTT_INCLUDE_DIR ${entt_INCLUDE_DIRS_DEBUG} )
set(EnTT_LIBRARIES ${entt_LIBRARIES_DEBUG} )
set(EnTT_DEFINITIONS ${entt_DEFINITIONS_DEBUG} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${entt_BUILD_MODULES_PATHS_DEBUG} )
    message(${EnTT_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


