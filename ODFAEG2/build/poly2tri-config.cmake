########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(poly2tri_FIND_QUIETLY)
    set(poly2tri_MESSAGE_MODE VERBOSE)
else()
    set(poly2tri_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/poly2triTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${poly2tri_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(poly2tri_VERSION_STRING "cci.20130502")
set(poly2tri_INCLUDE_DIRS ${poly2tri_INCLUDE_DIRS_RELEASE} )
set(poly2tri_INCLUDE_DIR ${poly2tri_INCLUDE_DIRS_RELEASE} )
set(poly2tri_LIBRARIES ${poly2tri_LIBRARIES_RELEASE} )
set(poly2tri_DEFINITIONS ${poly2tri_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${poly2tri_BUILD_MODULES_PATHS_RELEASE} )
    message(${poly2tri_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


