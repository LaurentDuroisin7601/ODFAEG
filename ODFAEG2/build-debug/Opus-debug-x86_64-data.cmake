########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND opus_COMPONENT_NAMES Opus::opus)
list(REMOVE_DUPLICATES opus_COMPONENT_NAMES)
if(DEFINED opus_FIND_DEPENDENCY_NAMES)
  list(APPEND opus_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES opus_FIND_DEPENDENCY_NAMES)
else()
  set(opus_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(opus_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/opusf83dc4e21f459/p")
set(opus_BUILD_MODULES_PATHS_DEBUG )


set(opus_INCLUDE_DIRS_DEBUG )
set(opus_RES_DIRS_DEBUG )
set(opus_DEFINITIONS_DEBUG )
set(opus_SHARED_LINK_FLAGS_DEBUG )
set(opus_EXE_LINK_FLAGS_DEBUG )
set(opus_OBJECTS_DEBUG )
set(opus_COMPILE_DEFINITIONS_DEBUG )
set(opus_COMPILE_OPTIONS_C_DEBUG )
set(opus_COMPILE_OPTIONS_CXX_DEBUG )
set(opus_LIB_DIRS_DEBUG "${opus_PACKAGE_FOLDER_DEBUG}/lib")
set(opus_BIN_DIRS_DEBUG )
set(opus_LIBRARY_TYPE_DEBUG STATIC)
set(opus_IS_HOST_WINDOWS_DEBUG 0)
set(opus_LIBS_DEBUG opus)
set(opus_SYSTEM_LIBS_DEBUG m)
set(opus_FRAMEWORK_DIRS_DEBUG )
set(opus_FRAMEWORKS_DEBUG )
set(opus_BUILD_DIRS_DEBUG )
set(opus_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(opus_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${opus_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${opus_COMPILE_OPTIONS_C_DEBUG}>")
set(opus_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${opus_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${opus_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${opus_EXE_LINK_FLAGS_DEBUG}>")


set(opus_COMPONENTS_DEBUG Opus::opus)
########### COMPONENT Opus::opus VARIABLES ############################################

set(opus_Opus_opus_INCLUDE_DIRS_DEBUG )
set(opus_Opus_opus_LIB_DIRS_DEBUG "${opus_PACKAGE_FOLDER_DEBUG}/lib")
set(opus_Opus_opus_BIN_DIRS_DEBUG )
set(opus_Opus_opus_LIBRARY_TYPE_DEBUG STATIC)
set(opus_Opus_opus_IS_HOST_WINDOWS_DEBUG 0)
set(opus_Opus_opus_RES_DIRS_DEBUG )
set(opus_Opus_opus_DEFINITIONS_DEBUG )
set(opus_Opus_opus_OBJECTS_DEBUG )
set(opus_Opus_opus_COMPILE_DEFINITIONS_DEBUG )
set(opus_Opus_opus_COMPILE_OPTIONS_C_DEBUG "")
set(opus_Opus_opus_COMPILE_OPTIONS_CXX_DEBUG "")
set(opus_Opus_opus_LIBS_DEBUG opus)
set(opus_Opus_opus_SYSTEM_LIBS_DEBUG m)
set(opus_Opus_opus_FRAMEWORK_DIRS_DEBUG )
set(opus_Opus_opus_FRAMEWORKS_DEBUG )
set(opus_Opus_opus_DEPENDENCIES_DEBUG )
set(opus_Opus_opus_SHARED_LINK_FLAGS_DEBUG )
set(opus_Opus_opus_EXE_LINK_FLAGS_DEBUG )
set(opus_Opus_opus_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(opus_Opus_opus_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${opus_Opus_opus_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${opus_Opus_opus_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${opus_Opus_opus_EXE_LINK_FLAGS_DEBUG}>
)
set(opus_Opus_opus_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${opus_Opus_opus_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${opus_Opus_opus_COMPILE_OPTIONS_C_DEBUG}>")