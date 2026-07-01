# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(stb_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(stb_FRAMEWORKS_FOUND_DEBUG "${stb_FRAMEWORKS_DEBUG}" "${stb_FRAMEWORK_DIRS_DEBUG}")

set(stb_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET stb_DEPS_TARGET)
    add_library(stb_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET stb_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${stb_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${stb_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### stb_DEPS_TARGET to all of them
conan_package_library_targets("${stb_LIBS_DEBUG}"    # libraries
                              "${stb_LIB_DIRS_DEBUG}" # package_libdir
                              "${stb_BIN_DIRS_DEBUG}" # package_bindir
                              "${stb_LIBRARY_TYPE_DEBUG}"
                              "${stb_IS_HOST_WINDOWS_DEBUG}"
                              stb_DEPS_TARGET
                              stb_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "stb"    # package_name
                              "${stb_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${stb_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET stb::stb
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${stb_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${stb_LIBRARIES_TARGETS}>
                 )

    if("${stb_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET stb::stb
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     stb_DEPS_TARGET)
    endif()

    set_property(TARGET stb::stb
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${stb_LINKER_FLAGS_DEBUG}>)
    set_property(TARGET stb::stb
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${stb_INCLUDE_DIRS_DEBUG}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET stb::stb
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${stb_LIB_DIRS_DEBUG}>)
    set_property(TARGET stb::stb
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${stb_COMPILE_DEFINITIONS_DEBUG}>)
    set_property(TARGET stb::stb
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${stb_COMPILE_OPTIONS_DEBUG}>)

########## For the modules (FindXXX)
set(stb_LIBRARIES_DEBUG stb::stb)
