########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(kuba-zip_COMPONENT_NAMES "")
if(DEFINED kuba-zip_FIND_DEPENDENCY_NAMES)
  list(APPEND kuba-zip_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES kuba-zip_FIND_DEPENDENCY_NAMES)
else()
  set(kuba-zip_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(kuba-zip_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/kuba-2d9a4c291581b/p")
set(kuba-zip_BUILD_MODULES_PATHS_DEBUG )


set(kuba-zip_INCLUDE_DIRS_DEBUG )
set(kuba-zip_RES_DIRS_DEBUG )
set(kuba-zip_DEFINITIONS_DEBUG )
set(kuba-zip_SHARED_LINK_FLAGS_DEBUG )
set(kuba-zip_EXE_LINK_FLAGS_DEBUG )
set(kuba-zip_OBJECTS_DEBUG )
set(kuba-zip_COMPILE_DEFINITIONS_DEBUG )
set(kuba-zip_COMPILE_OPTIONS_C_DEBUG )
set(kuba-zip_COMPILE_OPTIONS_CXX_DEBUG )
set(kuba-zip_LIB_DIRS_DEBUG "${kuba-zip_PACKAGE_FOLDER_DEBUG}/lib")
set(kuba-zip_BIN_DIRS_DEBUG )
set(kuba-zip_LIBRARY_TYPE_DEBUG STATIC)
set(kuba-zip_IS_HOST_WINDOWS_DEBUG 0)
set(kuba-zip_LIBS_DEBUG zip)
set(kuba-zip_SYSTEM_LIBS_DEBUG )
set(kuba-zip_FRAMEWORK_DIRS_DEBUG )
set(kuba-zip_FRAMEWORKS_DEBUG )
set(kuba-zip_BUILD_DIRS_DEBUG )
set(kuba-zip_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(kuba-zip_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${kuba-zip_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${kuba-zip_COMPILE_OPTIONS_C_DEBUG}>")
set(kuba-zip_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${kuba-zip_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${kuba-zip_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${kuba-zip_EXE_LINK_FLAGS_DEBUG}>")


set(kuba-zip_COMPONENTS_DEBUG )