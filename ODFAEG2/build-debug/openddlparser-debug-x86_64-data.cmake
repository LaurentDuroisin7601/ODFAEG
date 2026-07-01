########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(openddl-parser_COMPONENT_NAMES "")
if(DEFINED openddl-parser_FIND_DEPENDENCY_NAMES)
  list(APPEND openddl-parser_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES openddl-parser_FIND_DEPENDENCY_NAMES)
else()
  set(openddl-parser_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(openddl-parser_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/opend62c8f88073020/p")
set(openddl-parser_BUILD_MODULES_PATHS_DEBUG )


set(openddl-parser_INCLUDE_DIRS_DEBUG )
set(openddl-parser_RES_DIRS_DEBUG )
set(openddl-parser_DEFINITIONS_DEBUG "-DOPENDDL_STATIC_LIBARY")
set(openddl-parser_SHARED_LINK_FLAGS_DEBUG )
set(openddl-parser_EXE_LINK_FLAGS_DEBUG )
set(openddl-parser_OBJECTS_DEBUG )
set(openddl-parser_COMPILE_DEFINITIONS_DEBUG "OPENDDL_STATIC_LIBARY")
set(openddl-parser_COMPILE_OPTIONS_C_DEBUG )
set(openddl-parser_COMPILE_OPTIONS_CXX_DEBUG )
set(openddl-parser_LIB_DIRS_DEBUG "${openddl-parser_PACKAGE_FOLDER_DEBUG}/lib")
set(openddl-parser_BIN_DIRS_DEBUG )
set(openddl-parser_LIBRARY_TYPE_DEBUG STATIC)
set(openddl-parser_IS_HOST_WINDOWS_DEBUG 0)
set(openddl-parser_LIBS_DEBUG openddlparser)
set(openddl-parser_SYSTEM_LIBS_DEBUG m)
set(openddl-parser_FRAMEWORK_DIRS_DEBUG )
set(openddl-parser_FRAMEWORKS_DEBUG )
set(openddl-parser_BUILD_DIRS_DEBUG )
set(openddl-parser_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(openddl-parser_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${openddl-parser_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${openddl-parser_COMPILE_OPTIONS_C_DEBUG}>")
set(openddl-parser_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${openddl-parser_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${openddl-parser_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${openddl-parser_EXE_LINK_FLAGS_DEBUG}>")


set(openddl-parser_COMPONENTS_DEBUG )