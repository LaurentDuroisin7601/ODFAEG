########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(libmp3lame_COMPONENT_NAMES "")
if(DEFINED libmp3lame_FIND_DEPENDENCY_NAMES)
  list(APPEND libmp3lame_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES libmp3lame_FIND_DEPENDENCY_NAMES)
else()
  set(libmp3lame_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(libmp3lame_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/libmpadde42543df25/p")
set(libmp3lame_BUILD_MODULES_PATHS_DEBUG )


set(libmp3lame_INCLUDE_DIRS_DEBUG )
set(libmp3lame_RES_DIRS_DEBUG )
set(libmp3lame_DEFINITIONS_DEBUG )
set(libmp3lame_SHARED_LINK_FLAGS_DEBUG )
set(libmp3lame_EXE_LINK_FLAGS_DEBUG )
set(libmp3lame_OBJECTS_DEBUG )
set(libmp3lame_COMPILE_DEFINITIONS_DEBUG )
set(libmp3lame_COMPILE_OPTIONS_C_DEBUG )
set(libmp3lame_COMPILE_OPTIONS_CXX_DEBUG )
set(libmp3lame_LIB_DIRS_DEBUG "${libmp3lame_PACKAGE_FOLDER_DEBUG}/lib")
set(libmp3lame_BIN_DIRS_DEBUG )
set(libmp3lame_LIBRARY_TYPE_DEBUG STATIC)
set(libmp3lame_IS_HOST_WINDOWS_DEBUG 0)
set(libmp3lame_LIBS_DEBUG mp3lame)
set(libmp3lame_SYSTEM_LIBS_DEBUG m)
set(libmp3lame_FRAMEWORK_DIRS_DEBUG )
set(libmp3lame_FRAMEWORKS_DEBUG )
set(libmp3lame_BUILD_DIRS_DEBUG )
set(libmp3lame_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(libmp3lame_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${libmp3lame_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${libmp3lame_COMPILE_OPTIONS_C_DEBUG}>")
set(libmp3lame_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libmp3lame_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libmp3lame_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libmp3lame_EXE_LINK_FLAGS_DEBUG}>")


set(libmp3lame_COMPONENTS_DEBUG )