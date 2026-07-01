# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(openddl-parser_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(openddl-parser_FRAMEWORKS_FOUND_DEBUG "${openddl-parser_FRAMEWORKS_DEBUG}" "${openddl-parser_FRAMEWORK_DIRS_DEBUG}")

set(openddl-parser_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET openddl-parser_DEPS_TARGET)
    add_library(openddl-parser_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET openddl-parser_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${openddl-parser_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${openddl-parser_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### openddl-parser_DEPS_TARGET to all of them
conan_package_library_targets("${openddl-parser_LIBS_DEBUG}"    # libraries
                              "${openddl-parser_LIB_DIRS_DEBUG}" # package_libdir
                              "${openddl-parser_BIN_DIRS_DEBUG}" # package_bindir
                              "${openddl-parser_LIBRARY_TYPE_DEBUG}"
                              "${openddl-parser_IS_HOST_WINDOWS_DEBUG}"
                              openddl-parser_DEPS_TARGET
                              openddl-parser_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "openddl-parser"    # package_name
                              "${openddl-parser_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${openddl-parser_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${openddl-parser_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${openddl-parser_LIBRARIES_TARGETS}>
                 )

    if("${openddl-parser_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET openddlparser::openddlparser
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     openddl-parser_DEPS_TARGET)
    endif()

    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${openddl-parser_LINKER_FLAGS_DEBUG}>)
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${openddl-parser_INCLUDE_DIRS_DEBUG}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${openddl-parser_LIB_DIRS_DEBUG}>)
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${openddl-parser_COMPILE_DEFINITIONS_DEBUG}>)
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${openddl-parser_COMPILE_OPTIONS_DEBUG}>)

########## For the modules (FindXXX)
set(openddl-parser_LIBRARIES_DEBUG openddlparser::openddlparser)
