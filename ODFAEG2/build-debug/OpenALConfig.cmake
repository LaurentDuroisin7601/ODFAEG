########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(OpenAL_FIND_QUIETLY)
    set(OpenAL_MESSAGE_MODE VERBOSE)
else()
    set(OpenAL_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/OpenALTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${openal-soft_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(OpenAL_VERSION_STRING "1.23.1")
set(OpenAL_INCLUDE_DIRS ${openal-soft_INCLUDE_DIRS_DEBUG} )
set(OpenAL_INCLUDE_DIR ${openal-soft_INCLUDE_DIRS_DEBUG} )
set(OpenAL_LIBRARIES ${openal-soft_LIBRARIES_DEBUG} )
set(OpenAL_DEFINITIONS ${openal-soft_DEFINITIONS_DEBUG} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${openal-soft_BUILD_MODULES_PATHS_DEBUG} )
    message(${OpenAL_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


