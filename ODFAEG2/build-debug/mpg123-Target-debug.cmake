# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(mpg123_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(mpg123_FRAMEWORKS_FOUND_DEBUG "${mpg123_FRAMEWORKS_DEBUG}" "${mpg123_FRAMEWORK_DIRS_DEBUG}")

set(mpg123_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET mpg123_DEPS_TARGET)
    add_library(mpg123_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET mpg123_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${mpg123_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${mpg123_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:MPG123::libmpg123>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### mpg123_DEPS_TARGET to all of them
conan_package_library_targets("${mpg123_LIBS_DEBUG}"    # libraries
                              "${mpg123_LIB_DIRS_DEBUG}" # package_libdir
                              "${mpg123_BIN_DIRS_DEBUG}" # package_bindir
                              "${mpg123_LIBRARY_TYPE_DEBUG}"
                              "${mpg123_IS_HOST_WINDOWS_DEBUG}"
                              mpg123_DEPS_TARGET
                              mpg123_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "mpg123"    # package_name
                              "${mpg123_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${mpg123_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Debug ########################################

    ########## COMPONENT MPG123::libsyn123 #############

        set(mpg123_MPG123_libsyn123_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(mpg123_MPG123_libsyn123_FRAMEWORKS_FOUND_DEBUG "${mpg123_MPG123_libsyn123_FRAMEWORKS_DEBUG}" "${mpg123_MPG123_libsyn123_FRAMEWORK_DIRS_DEBUG}")

        set(mpg123_MPG123_libsyn123_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET mpg123_MPG123_libsyn123_DEPS_TARGET)
            add_library(mpg123_MPG123_libsyn123_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET mpg123_MPG123_libsyn123_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'mpg123_MPG123_libsyn123_DEPS_TARGET' to all of them
        conan_package_library_targets("${mpg123_MPG123_libsyn123_LIBS_DEBUG}"
                              "${mpg123_MPG123_libsyn123_LIB_DIRS_DEBUG}"
                              "${mpg123_MPG123_libsyn123_BIN_DIRS_DEBUG}" # package_bindir
                              "${mpg123_MPG123_libsyn123_LIBRARY_TYPE_DEBUG}"
                              "${mpg123_MPG123_libsyn123_IS_HOST_WINDOWS_DEBUG}"
                              mpg123_MPG123_libsyn123_DEPS_TARGET
                              mpg123_MPG123_libsyn123_LIBRARIES_TARGETS
                              "_DEBUG"
                              "mpg123_MPG123_libsyn123"
                              "${mpg123_MPG123_libsyn123_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET MPG123::libsyn123
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_LIBRARIES_TARGETS}>
                     )

        if("${mpg123_MPG123_libsyn123_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET MPG123::libsyn123
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         mpg123_MPG123_libsyn123_DEPS_TARGET)
        endif()

        set_property(TARGET MPG123::libsyn123 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET MPG123::libsyn123 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET MPG123::libsyn123 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_LIB_DIRS_DEBUG}>)
        set_property(TARGET MPG123::libsyn123 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET MPG123::libsyn123 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libsyn123_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT MPG123::libout123 #############

        set(mpg123_MPG123_libout123_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(mpg123_MPG123_libout123_FRAMEWORKS_FOUND_DEBUG "${mpg123_MPG123_libout123_FRAMEWORKS_DEBUG}" "${mpg123_MPG123_libout123_FRAMEWORK_DIRS_DEBUG}")

        set(mpg123_MPG123_libout123_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET mpg123_MPG123_libout123_DEPS_TARGET)
            add_library(mpg123_MPG123_libout123_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET mpg123_MPG123_libout123_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'mpg123_MPG123_libout123_DEPS_TARGET' to all of them
        conan_package_library_targets("${mpg123_MPG123_libout123_LIBS_DEBUG}"
                              "${mpg123_MPG123_libout123_LIB_DIRS_DEBUG}"
                              "${mpg123_MPG123_libout123_BIN_DIRS_DEBUG}" # package_bindir
                              "${mpg123_MPG123_libout123_LIBRARY_TYPE_DEBUG}"
                              "${mpg123_MPG123_libout123_IS_HOST_WINDOWS_DEBUG}"
                              mpg123_MPG123_libout123_DEPS_TARGET
                              mpg123_MPG123_libout123_LIBRARIES_TARGETS
                              "_DEBUG"
                              "mpg123_MPG123_libout123"
                              "${mpg123_MPG123_libout123_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET MPG123::libout123
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_LIBRARIES_TARGETS}>
                     )

        if("${mpg123_MPG123_libout123_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET MPG123::libout123
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         mpg123_MPG123_libout123_DEPS_TARGET)
        endif()

        set_property(TARGET MPG123::libout123 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET MPG123::libout123 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET MPG123::libout123 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_LIB_DIRS_DEBUG}>)
        set_property(TARGET MPG123::libout123 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET MPG123::libout123 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libout123_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT MPG123::libmpg123 #############

        set(mpg123_MPG123_libmpg123_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(mpg123_MPG123_libmpg123_FRAMEWORKS_FOUND_DEBUG "${mpg123_MPG123_libmpg123_FRAMEWORKS_DEBUG}" "${mpg123_MPG123_libmpg123_FRAMEWORK_DIRS_DEBUG}")

        set(mpg123_MPG123_libmpg123_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET mpg123_MPG123_libmpg123_DEPS_TARGET)
            add_library(mpg123_MPG123_libmpg123_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET mpg123_MPG123_libmpg123_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'mpg123_MPG123_libmpg123_DEPS_TARGET' to all of them
        conan_package_library_targets("${mpg123_MPG123_libmpg123_LIBS_DEBUG}"
                              "${mpg123_MPG123_libmpg123_LIB_DIRS_DEBUG}"
                              "${mpg123_MPG123_libmpg123_BIN_DIRS_DEBUG}" # package_bindir
                              "${mpg123_MPG123_libmpg123_LIBRARY_TYPE_DEBUG}"
                              "${mpg123_MPG123_libmpg123_IS_HOST_WINDOWS_DEBUG}"
                              mpg123_MPG123_libmpg123_DEPS_TARGET
                              mpg123_MPG123_libmpg123_LIBRARIES_TARGETS
                              "_DEBUG"
                              "mpg123_MPG123_libmpg123"
                              "${mpg123_MPG123_libmpg123_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET MPG123::libmpg123
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_LIBRARIES_TARGETS}>
                     )

        if("${mpg123_MPG123_libmpg123_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET MPG123::libmpg123
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         mpg123_MPG123_libmpg123_DEPS_TARGET)
        endif()

        set_property(TARGET MPG123::libmpg123 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET MPG123::libmpg123 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET MPG123::libmpg123 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_LIB_DIRS_DEBUG}>)
        set_property(TARGET MPG123::libmpg123 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET MPG123::libmpg123 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${mpg123_MPG123_libmpg123_COMPILE_OPTIONS_DEBUG}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET mpg123::mpg123 APPEND PROPERTY INTERFACE_LINK_LIBRARIES MPG123::libsyn123)
    set_property(TARGET mpg123::mpg123 APPEND PROPERTY INTERFACE_LINK_LIBRARIES MPG123::libout123)
    set_property(TARGET mpg123::mpg123 APPEND PROPERTY INTERFACE_LINK_LIBRARIES MPG123::libmpg123)

########## For the modules (FindXXX)
set(mpg123_LIBRARIES_DEBUG mpg123::mpg123)
