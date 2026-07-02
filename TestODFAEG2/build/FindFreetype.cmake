########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(Freetype_FIND_QUIETLY)
    set(Freetype_MESSAGE_MODE VERBOSE)
else()
    set(Freetype_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/module-FreetypeTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${freetype_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(FREETYPE_VERSION_STRING "2.13.2")
set(FREETYPE_INCLUDE_DIRS ${freetype_INCLUDE_DIRS_RELEASE} )
set(FREETYPE_INCLUDE_DIR ${freetype_INCLUDE_DIRS_RELEASE} )
set(FREETYPE_LIBRARIES ${freetype_LIBRARIES_RELEASE} )
set(FREETYPE_DEFINITIONS ${freetype_DEFINITIONS_RELEASE} )

set(Freetype_VERSION_STRING "2.13.2")
set(Freetype_INCLUDE_DIRS ${freetype_INCLUDE_DIRS_RELEASE} )
set(Freetype_INCLUDE_DIR ${freetype_INCLUDE_DIRS_RELEASE} )
set(Freetype_LIBRARIES ${freetype_LIBRARIES_RELEASE} )
set(Freetype_DEFINITIONS ${freetype_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${freetype_BUILD_MODULES_PATHS_RELEASE} )
    message(${Freetype_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


include(FindPackageHandleStandardArgs)
set(Freetype_FOUND 1)
set(Freetype_VERSION "2.13.2")

find_package_handle_standard_args(Freetype
                                  REQUIRED_VARS Freetype_VERSION
                                  VERSION_VAR Freetype_VERSION)
mark_as_advanced(Freetype_FOUND Freetype_VERSION)

set(FREETYPE_FOUND 1)
set(FREETYPE_VERSION "2.13.2")
mark_as_advanced(FREETYPE_FOUND FREETYPE_VERSION)
set(Freetype_FOUND 1)
set(Freetype_VERSION "2.13.2")
mark_as_advanced(Freetype_FOUND Freetype_VERSION)

