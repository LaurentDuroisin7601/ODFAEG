########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(clipper_COMPONENT_NAMES "")
if(DEFINED clipper_FIND_DEPENDENCY_NAMES)
  list(APPEND clipper_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES clipper_FIND_DEPENDENCY_NAMES)
else()
  set(clipper_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(clipper_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/clipp49f8bd5b55731/p")
set(clipper_BUILD_MODULES_PATHS_DEBUG )


set(clipper_INCLUDE_DIRS_DEBUG )
set(clipper_RES_DIRS_DEBUG )
set(clipper_DEFINITIONS_DEBUG )
set(clipper_SHARED_LINK_FLAGS_DEBUG )
set(clipper_EXE_LINK_FLAGS_DEBUG )
set(clipper_OBJECTS_DEBUG )
set(clipper_COMPILE_DEFINITIONS_DEBUG )
set(clipper_COMPILE_OPTIONS_C_DEBUG )
set(clipper_COMPILE_OPTIONS_CXX_DEBUG )
set(clipper_LIB_DIRS_DEBUG "${clipper_PACKAGE_FOLDER_DEBUG}/lib")
set(clipper_BIN_DIRS_DEBUG )
set(clipper_LIBRARY_TYPE_DEBUG STATIC)
set(clipper_IS_HOST_WINDOWS_DEBUG 0)
set(clipper_LIBS_DEBUG polyclipping)
set(clipper_SYSTEM_LIBS_DEBUG m)
set(clipper_FRAMEWORK_DIRS_DEBUG )
set(clipper_FRAMEWORKS_DEBUG )
set(clipper_BUILD_DIRS_DEBUG )
set(clipper_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(clipper_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${clipper_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${clipper_COMPILE_OPTIONS_C_DEBUG}>")
set(clipper_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${clipper_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${clipper_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${clipper_EXE_LINK_FLAGS_DEBUG}>")


set(clipper_COMPONENTS_DEBUG )