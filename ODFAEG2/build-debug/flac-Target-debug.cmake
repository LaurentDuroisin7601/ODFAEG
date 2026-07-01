# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(flac_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(flac_FRAMEWORKS_FOUND_DEBUG "${flac_FRAMEWORKS_DEBUG}" "${flac_FRAMEWORK_DIRS_DEBUG}")

set(flac_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET flac_DEPS_TARGET)
    add_library(flac_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET flac_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${flac_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${flac_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:Ogg::ogg;FLAC::FLAC>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### flac_DEPS_TARGET to all of them
conan_package_library_targets("${flac_LIBS_DEBUG}"    # libraries
                              "${flac_LIB_DIRS_DEBUG}" # package_libdir
                              "${flac_BIN_DIRS_DEBUG}" # package_bindir
                              "${flac_LIBRARY_TYPE_DEBUG}"
                              "${flac_IS_HOST_WINDOWS_DEBUG}"
                              flac_DEPS_TARGET
                              flac_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "flac"    # package_name
                              "${flac_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${flac_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Debug ########################################

    ########## COMPONENT FLAC::FLAC++ #############

        set(flac_FLAC_FLAC++_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(flac_FLAC_FLAC++_FRAMEWORKS_FOUND_DEBUG "${flac_FLAC_FLAC++_FRAMEWORKS_DEBUG}" "${flac_FLAC_FLAC++_FRAMEWORK_DIRS_DEBUG}")

        set(flac_FLAC_FLAC++_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET flac_FLAC_FLAC++_DEPS_TARGET)
            add_library(flac_FLAC_FLAC++_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET flac_FLAC_FLAC++_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'flac_FLAC_FLAC++_DEPS_TARGET' to all of them
        conan_package_library_targets("${flac_FLAC_FLAC++_LIBS_DEBUG}"
                              "${flac_FLAC_FLAC++_LIB_DIRS_DEBUG}"
                              "${flac_FLAC_FLAC++_BIN_DIRS_DEBUG}" # package_bindir
                              "${flac_FLAC_FLAC++_LIBRARY_TYPE_DEBUG}"
                              "${flac_FLAC_FLAC++_IS_HOST_WINDOWS_DEBUG}"
                              flac_FLAC_FLAC++_DEPS_TARGET
                              flac_FLAC_FLAC++_LIBRARIES_TARGETS
                              "_DEBUG"
                              "flac_FLAC_FLAC++"
                              "${flac_FLAC_FLAC++_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET FLAC::FLAC++
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_LIBRARIES_TARGETS}>
                     )

        if("${flac_FLAC_FLAC++_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET FLAC::FLAC++
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         flac_FLAC_FLAC++_DEPS_TARGET)
        endif()

        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_LIB_DIRS_DEBUG}>)
        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET FLAC::FLAC++ APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC++_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT FLAC::FLAC #############

        set(flac_FLAC_FLAC_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(flac_FLAC_FLAC_FRAMEWORKS_FOUND_DEBUG "${flac_FLAC_FLAC_FRAMEWORKS_DEBUG}" "${flac_FLAC_FLAC_FRAMEWORK_DIRS_DEBUG}")

        set(flac_FLAC_FLAC_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET flac_FLAC_FLAC_DEPS_TARGET)
            add_library(flac_FLAC_FLAC_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET flac_FLAC_FLAC_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'flac_FLAC_FLAC_DEPS_TARGET' to all of them
        conan_package_library_targets("${flac_FLAC_FLAC_LIBS_DEBUG}"
                              "${flac_FLAC_FLAC_LIB_DIRS_DEBUG}"
                              "${flac_FLAC_FLAC_BIN_DIRS_DEBUG}" # package_bindir
                              "${flac_FLAC_FLAC_LIBRARY_TYPE_DEBUG}"
                              "${flac_FLAC_FLAC_IS_HOST_WINDOWS_DEBUG}"
                              flac_FLAC_FLAC_DEPS_TARGET
                              flac_FLAC_FLAC_LIBRARIES_TARGETS
                              "_DEBUG"
                              "flac_FLAC_FLAC"
                              "${flac_FLAC_FLAC_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET FLAC::FLAC
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_LIBRARIES_TARGETS}>
                     )

        if("${flac_FLAC_FLAC_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET FLAC::FLAC
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         flac_FLAC_FLAC_DEPS_TARGET)
        endif()

        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_LIB_DIRS_DEBUG}>)
        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET FLAC::FLAC APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${flac_FLAC_FLAC_COMPILE_OPTIONS_DEBUG}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET flac::flac APPEND PROPERTY INTERFACE_LINK_LIBRARIES FLAC::FLAC++)
    set_property(TARGET flac::flac APPEND PROPERTY INTERFACE_LINK_LIBRARIES FLAC::FLAC)

########## For the modules (FindXXX)
set(flac_LIBRARIES_DEBUG flac::flac)
