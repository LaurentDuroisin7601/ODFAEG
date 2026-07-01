# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(openal-soft_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(openal-soft_FRAMEWORKS_FOUND_DEBUG "${openal-soft_FRAMEWORKS_DEBUG}" "${openal-soft_FRAMEWORK_DIRS_DEBUG}")

set(openal-soft_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET openal-soft_DEPS_TARGET)
    add_library(openal-soft_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET openal-soft_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${openal-soft_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${openal-soft_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:ALSA::ALSA>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### openal-soft_DEPS_TARGET to all of them
conan_package_library_targets("${openal-soft_LIBS_DEBUG}"    # libraries
                              "${openal-soft_LIB_DIRS_DEBUG}" # package_libdir
                              "${openal-soft_BIN_DIRS_DEBUG}" # package_bindir
                              "${openal-soft_LIBRARY_TYPE_DEBUG}"
                              "${openal-soft_IS_HOST_WINDOWS_DEBUG}"
                              openal-soft_DEPS_TARGET
                              openal-soft_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "openal-soft"    # package_name
                              "${openal-soft_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${openal-soft_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET OpenAL::OpenAL
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${openal-soft_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${openal-soft_LIBRARIES_TARGETS}>
                 )

    if("${openal-soft_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET OpenAL::OpenAL
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     openal-soft_DEPS_TARGET)
    endif()

    set_property(TARGET OpenAL::OpenAL
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${openal-soft_LINKER_FLAGS_DEBUG}>)
    set_property(TARGET OpenAL::OpenAL
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${openal-soft_INCLUDE_DIRS_DEBUG}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET OpenAL::OpenAL
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${openal-soft_LIB_DIRS_DEBUG}>)
    set_property(TARGET OpenAL::OpenAL
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${openal-soft_COMPILE_DEFINITIONS_DEBUG}>)
    set_property(TARGET OpenAL::OpenAL
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${openal-soft_COMPILE_OPTIONS_DEBUG}>)

########## For the modules (FindXXX)
set(openal-soft_LIBRARIES_DEBUG OpenAL::OpenAL)
