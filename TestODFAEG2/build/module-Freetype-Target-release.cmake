# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(freetype_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(freetype_FRAMEWORKS_FOUND_RELEASE "${freetype_FRAMEWORKS_RELEASE}" "${freetype_FRAMEWORK_DIRS_RELEASE}")

set(freetype_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET freetype_DEPS_TARGET)
    add_library(freetype_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET freetype_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${freetype_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${freetype_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:PNG::PNG;brotli::brotli;ZLIB::ZLIB;BZip2::BZip2>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### freetype_DEPS_TARGET to all of them
conan_package_library_targets("${freetype_LIBS_RELEASE}"    # libraries
                              "${freetype_LIB_DIRS_RELEASE}" # package_libdir
                              "${freetype_BIN_DIRS_RELEASE}" # package_bindir
                              "${freetype_LIBRARY_TYPE_RELEASE}"
                              "${freetype_IS_HOST_WINDOWS_RELEASE}"
                              freetype_DEPS_TARGET
                              freetype_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "freetype"    # package_name
                              "${freetype_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${freetype_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${freetype_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${freetype_LIBRARIES_TARGETS}>
                 )

    if("${freetype_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET Freetype::Freetype
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     freetype_DEPS_TARGET)
    endif()

    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${freetype_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${freetype_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${freetype_LIB_DIRS_RELEASE}>)
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${freetype_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET Freetype::Freetype
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${freetype_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(freetype_LIBRARIES_RELEASE Freetype::Freetype)
