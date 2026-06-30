# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(assimp_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(assimp_FRAMEWORKS_FOUND_RELEASE "${assimp_FRAMEWORKS_RELEASE}" "${assimp_FRAMEWORK_DIRS_RELEASE}")

set(assimp_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET assimp_DEPS_TARGET)
    add_library(assimp_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET assimp_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${assimp_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${assimp_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:minizip::minizip;pugixml::pugixml;zip::zip;poly2tri::poly2tri;draco::draco;clipper::clipper;openddlparser::openddlparser;ZLIB::ZLIB;stb::stb>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### assimp_DEPS_TARGET to all of them
conan_package_library_targets("${assimp_LIBS_RELEASE}"    # libraries
                              "${assimp_LIB_DIRS_RELEASE}" # package_libdir
                              "${assimp_BIN_DIRS_RELEASE}" # package_bindir
                              "${assimp_LIBRARY_TYPE_RELEASE}"
                              "${assimp_IS_HOST_WINDOWS_RELEASE}"
                              assimp_DEPS_TARGET
                              assimp_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "assimp"    # package_name
                              "${assimp_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${assimp_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${assimp_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${assimp_LIBRARIES_TARGETS}>
                 )

    if("${assimp_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET assimp::assimp
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     assimp_DEPS_TARGET)
    endif()

    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${assimp_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${assimp_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${assimp_LIB_DIRS_RELEASE}>)
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${assimp_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${assimp_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(assimp_LIBRARIES_RELEASE assimp::assimp)
