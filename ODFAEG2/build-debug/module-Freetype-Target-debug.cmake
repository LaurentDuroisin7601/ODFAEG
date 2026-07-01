# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(freetype_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(freetype_FRAMEWORKS_FOUND_DEBUG "${freetype_FRAMEWORKS_DEBUG}" "${freetype_FRAMEWORK_DIRS_DEBUG}")

set(freetype_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET freetype_DEPS_TARGET)
    add_library(freetype_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET freetype_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${freetype_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${freetype_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:PNG::PNG;brotli::brotli;ZLIB::ZLIB;BZip2::BZip2>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### freetype_DEPS_TARGET to all of them
conan_package_library_targets("${freetype_LIBS_DEBUG}"    # libraries
                              "${freetype_LIB_DIRS_DEBUG}" # package_libdir
                              "${freetype_BIN_DIRS_DEBUG}" # package_bindir
                              "${freetype_LIBRARY_TYPE_DEBUG}"
                              "${freetype_IS_HOST_WINDOWS_DEBUG}"
                              freetype_DEPS_TARGET
                              freetype_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "freetype"    # package_name
                              "${freetype_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${freetype_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${freetype_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${freetype_LIBRARIES_TARGETS}>
                 )

    if("${freetype_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET Freetype::Freetype
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     freetype_DEPS_TARGET)
    endif()

    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${freetype_LINKER_FLAGS_DEBUG}>)
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${freetype_INCLUDE_DIRS_DEBUG}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${freetype_LIB_DIRS_DEBUG}>)
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${freetype_COMPILE_DEFINITIONS_DEBUG}>)
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${freetype_COMPILE_OPTIONS_DEBUG}>)

########## For the modules (FindXXX)
set(freetype_LIBRARIES_DEBUG Freetype::Freetype)
