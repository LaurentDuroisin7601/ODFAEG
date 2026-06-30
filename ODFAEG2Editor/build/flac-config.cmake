########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(flac_FIND_QUIETLY)
    set(flac_MESSAGE_MODE VERBOSE)
else()
    set(flac_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/flacTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${flac_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(flac_VERSION_STRING "1.4.2")
set(flac_INCLUDE_DIRS ${flac_INCLUDE_DIRS_RELEASE} )
set(flac_INCLUDE_DIR ${flac_INCLUDE_DIRS_RELEASE} )
set(flac_LIBRARIES ${flac_LIBRARIES_RELEASE} )
set(flac_DEFINITIONS ${flac_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${flac_BUILD_MODULES_PATHS_RELEASE} )
    message(${flac_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


