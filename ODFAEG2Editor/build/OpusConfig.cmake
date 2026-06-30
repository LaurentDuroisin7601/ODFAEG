########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(Opus_FIND_QUIETLY)
    set(Opus_MESSAGE_MODE VERBOSE)
else()
    set(Opus_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/OpusTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${opus_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(Opus_VERSION_STRING "1.6.1")
set(Opus_INCLUDE_DIRS ${opus_INCLUDE_DIRS_RELEASE} )
set(Opus_INCLUDE_DIR ${opus_INCLUDE_DIRS_RELEASE} )
set(Opus_LIBRARIES ${opus_LIBRARIES_RELEASE} )
set(Opus_DEFINITIONS ${opus_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${opus_BUILD_MODULES_PATHS_RELEASE} )
    message(${Opus_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


