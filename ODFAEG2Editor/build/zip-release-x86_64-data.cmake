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
set(kuba-zip_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/kuba-87c04591aee84/p")
set(kuba-zip_BUILD_MODULES_PATHS_RELEASE )


set(kuba-zip_INCLUDE_DIRS_RELEASE )
set(kuba-zip_RES_DIRS_RELEASE )
set(kuba-zip_DEFINITIONS_RELEASE )
set(kuba-zip_SHARED_LINK_FLAGS_RELEASE )
set(kuba-zip_EXE_LINK_FLAGS_RELEASE )
set(kuba-zip_OBJECTS_RELEASE )
set(kuba-zip_COMPILE_DEFINITIONS_RELEASE )
set(kuba-zip_COMPILE_OPTIONS_C_RELEASE )
set(kuba-zip_COMPILE_OPTIONS_CXX_RELEASE )
set(kuba-zip_LIB_DIRS_RELEASE "${kuba-zip_PACKAGE_FOLDER_RELEASE}/lib")
set(kuba-zip_BIN_DIRS_RELEASE )
set(kuba-zip_LIBRARY_TYPE_RELEASE STATIC)
set(kuba-zip_IS_HOST_WINDOWS_RELEASE 0)
set(kuba-zip_LIBS_RELEASE zip)
set(kuba-zip_SYSTEM_LIBS_RELEASE )
set(kuba-zip_FRAMEWORK_DIRS_RELEASE )
set(kuba-zip_FRAMEWORKS_RELEASE )
set(kuba-zip_BUILD_DIRS_RELEASE )
set(kuba-zip_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(kuba-zip_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${kuba-zip_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${kuba-zip_COMPILE_OPTIONS_C_RELEASE}>")
set(kuba-zip_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${kuba-zip_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${kuba-zip_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${kuba-zip_EXE_LINK_FLAGS_RELEASE}>")


set(kuba-zip_COMPONENTS_RELEASE )