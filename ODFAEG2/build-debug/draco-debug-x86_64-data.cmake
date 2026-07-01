########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(draco_COMPONENT_NAMES "")
if(DEFINED draco_FIND_DEPENDENCY_NAMES)
  list(APPEND draco_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES draco_FIND_DEPENDENCY_NAMES)
else()
  set(draco_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(draco_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/draco9e7dd8d1bb398/p")
set(draco_BUILD_MODULES_PATHS_DEBUG )


set(draco_INCLUDE_DIRS_DEBUG )
set(draco_RES_DIRS_DEBUG )
set(draco_DEFINITIONS_DEBUG )
set(draco_SHARED_LINK_FLAGS_DEBUG )
set(draco_EXE_LINK_FLAGS_DEBUG )
set(draco_OBJECTS_DEBUG )
set(draco_COMPILE_DEFINITIONS_DEBUG )
set(draco_COMPILE_OPTIONS_C_DEBUG )
set(draco_COMPILE_OPTIONS_CXX_DEBUG )
set(draco_LIB_DIRS_DEBUG "${draco_PACKAGE_FOLDER_DEBUG}/lib")
set(draco_BIN_DIRS_DEBUG )
set(draco_LIBRARY_TYPE_DEBUG STATIC)
set(draco_IS_HOST_WINDOWS_DEBUG 0)
set(draco_LIBS_DEBUG draco)
set(draco_SYSTEM_LIBS_DEBUG m)
set(draco_FRAMEWORK_DIRS_DEBUG )
set(draco_FRAMEWORKS_DEBUG )
set(draco_BUILD_DIRS_DEBUG )
set(draco_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(draco_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${draco_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${draco_COMPILE_OPTIONS_C_DEBUG}>")
set(draco_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${draco_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${draco_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${draco_EXE_LINK_FLAGS_DEBUG}>")


set(draco_COMPONENTS_DEBUG )