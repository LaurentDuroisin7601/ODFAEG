########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(libmp3lame_FIND_QUIETLY)
    set(libmp3lame_MESSAGE_MODE VERBOSE)
else()
    set(libmp3lame_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/libmp3lameTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${libmp3lame_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(libmp3lame_VERSION_STRING "3.100")
set(libmp3lame_INCLUDE_DIRS ${libmp3lame_INCLUDE_DIRS_RELEASE} )
set(libmp3lame_INCLUDE_DIR ${libmp3lame_INCLUDE_DIRS_RELEASE} )
set(libmp3lame_LIBRARIES ${libmp3lame_LIBRARIES_RELEASE} )
set(libmp3lame_DEFINITIONS ${libmp3lame_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${libmp3lame_BUILD_MODULES_PATHS_RELEASE} )
    message(${libmp3lame_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


