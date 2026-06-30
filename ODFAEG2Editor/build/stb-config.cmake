########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(stb_FIND_QUIETLY)
    set(stb_MESSAGE_MODE VERBOSE)
else()
    set(stb_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/stbTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${stb_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(stb_VERSION_STRING "cci.20230920")
set(stb_INCLUDE_DIRS ${stb_INCLUDE_DIRS_RELEASE} )
set(stb_INCLUDE_DIR ${stb_INCLUDE_DIRS_RELEASE} )
set(stb_LIBRARIES ${stb_LIBRARIES_RELEASE} )
set(stb_DEFINITIONS ${stb_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${stb_BUILD_MODULES_PATHS_RELEASE} )
    message(${stb_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


