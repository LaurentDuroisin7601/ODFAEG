########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(zip_FIND_QUIETLY)
    set(zip_MESSAGE_MODE VERBOSE)
else()
    set(zip_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/zipTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${kuba-zip_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(zip_VERSION_STRING "0.3.0")
set(zip_INCLUDE_DIRS ${kuba-zip_INCLUDE_DIRS_RELEASE} )
set(zip_INCLUDE_DIR ${kuba-zip_INCLUDE_DIRS_RELEASE} )
set(zip_LIBRARIES ${kuba-zip_LIBRARIES_RELEASE} )
set(zip_DEFINITIONS ${kuba-zip_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${kuba-zip_BUILD_MODULES_PATHS_RELEASE} )
    message(${zip_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


