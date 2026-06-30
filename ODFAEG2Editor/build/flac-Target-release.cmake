# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(flac_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(flac_FRAMEWORKS_FOUND_RELEASE "${flac_FRAMEWORKS_RELEASE}" "${flac_FRAMEWORK_DIRS_RELEASE}")

set(flac_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET flac_DEPS_TARGET)
    add_library(flac_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET flac_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${flac_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${flac_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:Ogg::ogg;FLAC::FLAC>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### flac_DEPS_TARGET to all of them
conan_package_library_targets("${flac_LIBS_RELEASE}"    # libraries
                              "${flac_LIB_DIRS_RELEASE}" # package_libdir
                              "${flac_BIN_DIRS_RELEASE}" # package_bindir
                              "${flac_LIBRARY_TYPE_RELEASE}"
                              "${flac_IS_HOST_WINDOWS_RELEASE}"
                              flac_DEPS_TARGET
                              flac_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "flac"    # package_name
                              "${flac_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${flac_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT FLAC::FLAC++ #############

        set(flac_FLAC_FLAC++_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(flac_FLAC_FLAC++_FRAMEWORKS_FOUND_RELEASE "${flac_FLAC_FLAC++_FRAMEWORKS_RELEASE}" "${flac_FLAC_FLAC++_FRAMEWORK_DIRS_RELEASE}")

        set(flac_FLAC_FLAC++_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET flac_FLAC_FLAC++_DEPS_TARGET)
            add_library(flac_FLAC_FLAC++_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET flac_FLAC_FLAC++_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'flac_FLAC_FLAC++_DEPS_TARGET' to all of them
        conan_package_library_targets("${flac_FLAC_FLAC++_LIBS_RELEASE}"
                              "${flac_FLAC_FLAC++_LIB_DIRS_RELEASE}"
                              "${flac_FLAC_FLAC++_BIN_DIRS_RELEASE}" # package_bindir
                              "${flac_FLAC_FLAC++_LIBRARY_TYPE_RELEASE}"
                              "${flac_FLAC_FLAC++_IS_HOST_WINDOWS_RELEASE}"
                              flac_FLAC_FLAC++_DEPS_TARGET
                              flac_FLAC_FLAC++_LIBRARIES_TARGETS
                              "_RELEASE"
                              "flac_FLAC_FLAC++"
                              "${flac_FLAC_FLAC++_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET FLAC::FLAC++
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_LIBRARIES_TARGETS}>
                     )

        if("${flac_FLAC_FLAC++_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET FLAC::FLAC++
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         flac_FLAC_FLAC++_DEPS_TARGET)
        endif()

        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_LIB_DIRS_RELEASE}>)
        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC++_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT FLAC::FLAC #############

        set(flac_FLAC_FLAC_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(flac_FLAC_FLAC_FRAMEWORKS_FOUND_RELEASE "${flac_FLAC_FLAC_FRAMEWORKS_RELEASE}" "${flac_FLAC_FLAC_FRAMEWORK_DIRS_RELEASE}")

        set(flac_FLAC_FLAC_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET flac_FLAC_FLAC_DEPS_TARGET)
            add_library(flac_FLAC_FLAC_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET flac_FLAC_FLAC_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'flac_FLAC_FLAC_DEPS_TARGET' to all of them
        conan_package_library_targets("${flac_FLAC_FLAC_LIBS_RELEASE}"
                              "${flac_FLAC_FLAC_LIB_DIRS_RELEASE}"
                              "${flac_FLAC_FLAC_BIN_DIRS_RELEASE}" # package_bindir
                              "${flac_FLAC_FLAC_LIBRARY_TYPE_RELEASE}"
                              "${flac_FLAC_FLAC_IS_HOST_WINDOWS_RELEASE}"
                              flac_FLAC_FLAC_DEPS_TARGET
                              flac_FLAC_FLAC_LIBRARIES_TARGETS
                              "_RELEASE"
                              "flac_FLAC_FLAC"
                              "${flac_FLAC_FLAC_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET FLAC::FLAC
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_LIBRARIES_TARGETS}>
                     )

        if("${flac_FLAC_FLAC_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET FLAC::FLAC
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         flac_FLAC_FLAC_DEPS_TARGET)
        endif()

        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_LIB_DIRS_RELEASE}>)
        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${flac_FLAC_FLAC_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET flac::flac APPEND PROPERTY INTERFACE_LINK_LIBRARIES FLAC::FLAC++)
    set_property(TARGET flac::flac APPEND PROPERTY INTERFACE_LINK_LIBRARIES FLAC::FLAC)

########## For the modules (FindXXX)
set(flac_LIBRARIES_RELEASE flac::flac)
