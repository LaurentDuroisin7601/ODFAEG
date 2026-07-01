########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(mpg123_FIND_QUIETLY)
    set(mpg123_MESSAGE_MODE VERBOSE)
else()
    set(mpg123_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/mpg123Targets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${mpg123_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(mpg123_VERSION_STRING "1.33.0")
set(mpg123_INCLUDE_DIRS ${mpg123_INCLUDE_DIRS_DEBUG} )
set(mpg123_INCLUDE_DIR ${mpg123_INCLUDE_DIRS_DEBUG} )
set(mpg123_LIBRARIES ${mpg123_LIBRARIES_DEBUG} )
set(mpg123_DEFINITIONS ${mpg123_DEFINITIONS_DEBUG} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${mpg123_BUILD_MODULES_PATHS_DEBUG} )
    message(${mpg123_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


