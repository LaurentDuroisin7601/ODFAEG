########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(entt_COMPONENT_NAMES "")
if(DEFINED entt_FIND_DEPENDENCY_NAMES)
  list(APPEND entt_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES entt_FIND_DEPENDENCY_NAMES)
else()
  set(entt_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(entt_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/enttb1832d5c14fb1/p")
set(entt_BUILD_MODULES_PATHS_RELEASE )


set(entt_INCLUDE_DIRS_RELEASE "${entt_PACKAGE_FOLDER_RELEASE}/include")
set(entt_RES_DIRS_RELEASE )
set(entt_DEFINITIONS_RELEASE )
set(entt_SHARED_LINK_FLAGS_RELEASE )
set(entt_EXE_LINK_FLAGS_RELEASE )
set(entt_OBJECTS_RELEASE )
set(entt_COMPILE_DEFINITIONS_RELEASE )
set(entt_COMPILE_OPTIONS_C_RELEASE )
set(entt_COMPILE_OPTIONS_CXX_RELEASE )
set(entt_LIB_DIRS_RELEASE )
set(entt_BIN_DIRS_RELEASE )
set(entt_LIBRARY_TYPE_RELEASE UNKNOWN)
set(entt_IS_HOST_WINDOWS_RELEASE 0)
set(entt_LIBS_RELEASE )
set(entt_SYSTEM_LIBS_RELEASE )
set(entt_FRAMEWORK_DIRS_RELEASE )
set(entt_FRAMEWORKS_RELEASE )
set(entt_BUILD_DIRS_RELEASE )
set(entt_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(entt_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${entt_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${entt_COMPILE_OPTIONS_C_RELEASE}>")
set(entt_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${entt_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${entt_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${entt_EXE_LINK_FLAGS_RELEASE}>")


set(entt_COMPONENTS_RELEASE )