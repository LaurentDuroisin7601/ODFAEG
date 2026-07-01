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
set(entt_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/enttb1832d5c14fb1/p")
set(entt_BUILD_MODULES_PATHS_DEBUG )


set(entt_INCLUDE_DIRS_DEBUG "${entt_PACKAGE_FOLDER_DEBUG}/include")
set(entt_RES_DIRS_DEBUG )
set(entt_DEFINITIONS_DEBUG )
set(entt_SHARED_LINK_FLAGS_DEBUG )
set(entt_EXE_LINK_FLAGS_DEBUG )
set(entt_OBJECTS_DEBUG )
set(entt_COMPILE_DEFINITIONS_DEBUG )
set(entt_COMPILE_OPTIONS_C_DEBUG )
set(entt_COMPILE_OPTIONS_CXX_DEBUG )
set(entt_LIB_DIRS_DEBUG )
set(entt_BIN_DIRS_DEBUG )
set(entt_LIBRARY_TYPE_DEBUG UNKNOWN)
set(entt_IS_HOST_WINDOWS_DEBUG 0)
set(entt_LIBS_DEBUG )
set(entt_SYSTEM_LIBS_DEBUG )
set(entt_FRAMEWORK_DIRS_DEBUG )
set(entt_FRAMEWORKS_DEBUG )
set(entt_BUILD_DIRS_DEBUG )
set(entt_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(entt_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${entt_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${entt_COMPILE_OPTIONS_C_DEBUG}>")
set(entt_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${entt_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${entt_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${entt_EXE_LINK_FLAGS_DEBUG}>")


set(entt_COMPONENTS_DEBUG )