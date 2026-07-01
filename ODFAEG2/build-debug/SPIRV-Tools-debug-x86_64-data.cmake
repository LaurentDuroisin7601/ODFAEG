########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND spirv-tools_COMPONENT_NAMES SPIRV-Tools-static SPIRV-Tools-opt SPIRV-Tools-link SPIRV-Tools-reduce SPIRV-Tools-lint SPIRV-Tools-diff)
list(REMOVE_DUPLICATES spirv-tools_COMPONENT_NAMES)
if(DEFINED spirv-tools_FIND_DEPENDENCY_NAMES)
  list(APPEND spirv-tools_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES spirv-tools_FIND_DEPENDENCY_NAMES)
else()
  set(spirv-tools_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(spirv-tools_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/spirv31773ad967e44/p")
set(spirv-tools_BUILD_MODULES_PATHS_DEBUG )


set(spirv-tools_INCLUDE_DIRS_DEBUG )
set(spirv-tools_RES_DIRS_DEBUG )
set(spirv-tools_DEFINITIONS_DEBUG )
set(spirv-tools_SHARED_LINK_FLAGS_DEBUG )
set(spirv-tools_EXE_LINK_FLAGS_DEBUG )
set(spirv-tools_OBJECTS_DEBUG )
set(spirv-tools_COMPILE_DEFINITIONS_DEBUG )
set(spirv-tools_COMPILE_OPTIONS_C_DEBUG )
set(spirv-tools_COMPILE_OPTIONS_CXX_DEBUG )
set(spirv-tools_LIB_DIRS_DEBUG "${spirv-tools_PACKAGE_FOLDER_DEBUG}/lib")
set(spirv-tools_BIN_DIRS_DEBUG )
set(spirv-tools_LIBRARY_TYPE_DEBUG STATIC)
set(spirv-tools_IS_HOST_WINDOWS_DEBUG 0)
set(spirv-tools_LIBS_DEBUG SPIRV-Tools-diff SPIRV-Tools-lint SPIRV-Tools-reduce SPIRV-Tools-link SPIRV-Tools-opt SPIRV-Tools)
set(spirv-tools_SYSTEM_LIBS_DEBUG m rt stdc++)
set(spirv-tools_FRAMEWORK_DIRS_DEBUG )
set(spirv-tools_FRAMEWORKS_DEBUG )
set(spirv-tools_BUILD_DIRS_DEBUG )
set(spirv-tools_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(spirv-tools_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${spirv-tools_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${spirv-tools_COMPILE_OPTIONS_C_DEBUG}>")
set(spirv-tools_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${spirv-tools_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${spirv-tools_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${spirv-tools_EXE_LINK_FLAGS_DEBUG}>")


set(spirv-tools_COMPONENTS_DEBUG SPIRV-Tools-static SPIRV-Tools-opt SPIRV-Tools-link SPIRV-Tools-reduce SPIRV-Tools-lint SPIRV-Tools-diff)
########### COMPONENT SPIRV-Tools-diff VARIABLES ############################################

set(spirv-tools_SPIRV-Tools-diff_INCLUDE_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_LIB_DIRS_DEBUG "${spirv-tools_PACKAGE_FOLDER_DEBUG}/lib")
set(spirv-tools_SPIRV-Tools-diff_BIN_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_LIBRARY_TYPE_DEBUG STATIC)
set(spirv-tools_SPIRV-Tools-diff_IS_HOST_WINDOWS_DEBUG 0)
set(spirv-tools_SPIRV-Tools-diff_RES_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_OBJECTS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_COMPILE_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_COMPILE_OPTIONS_C_DEBUG "")
set(spirv-tools_SPIRV-Tools-diff_COMPILE_OPTIONS_CXX_DEBUG "")
set(spirv-tools_SPIRV-Tools-diff_LIBS_DEBUG SPIRV-Tools-diff)
set(spirv-tools_SPIRV-Tools-diff_SYSTEM_LIBS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_FRAMEWORK_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_FRAMEWORKS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_DEPENDENCIES_DEBUG SPIRV-Tools-static SPIRV-Tools-opt)
set(spirv-tools_SPIRV-Tools-diff_SHARED_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_EXE_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-diff_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(spirv-tools_SPIRV-Tools-diff_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${spirv-tools_SPIRV-Tools-diff_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${spirv-tools_SPIRV-Tools-diff_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${spirv-tools_SPIRV-Tools-diff_EXE_LINK_FLAGS_DEBUG}>
)
set(spirv-tools_SPIRV-Tools-diff_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${spirv-tools_SPIRV-Tools-diff_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${spirv-tools_SPIRV-Tools-diff_COMPILE_OPTIONS_C_DEBUG}>")
########### COMPONENT SPIRV-Tools-lint VARIABLES ############################################

set(spirv-tools_SPIRV-Tools-lint_INCLUDE_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_LIB_DIRS_DEBUG "${spirv-tools_PACKAGE_FOLDER_DEBUG}/lib")
set(spirv-tools_SPIRV-Tools-lint_BIN_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_LIBRARY_TYPE_DEBUG STATIC)
set(spirv-tools_SPIRV-Tools-lint_IS_HOST_WINDOWS_DEBUG 0)
set(spirv-tools_SPIRV-Tools-lint_RES_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_OBJECTS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_COMPILE_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_COMPILE_OPTIONS_C_DEBUG "")
set(spirv-tools_SPIRV-Tools-lint_COMPILE_OPTIONS_CXX_DEBUG "")
set(spirv-tools_SPIRV-Tools-lint_LIBS_DEBUG SPIRV-Tools-lint)
set(spirv-tools_SPIRV-Tools-lint_SYSTEM_LIBS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_FRAMEWORK_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_FRAMEWORKS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_DEPENDENCIES_DEBUG SPIRV-Tools-static SPIRV-Tools-opt)
set(spirv-tools_SPIRV-Tools-lint_SHARED_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_EXE_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-lint_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(spirv-tools_SPIRV-Tools-lint_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${spirv-tools_SPIRV-Tools-lint_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${spirv-tools_SPIRV-Tools-lint_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${spirv-tools_SPIRV-Tools-lint_EXE_LINK_FLAGS_DEBUG}>
)
set(spirv-tools_SPIRV-Tools-lint_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${spirv-tools_SPIRV-Tools-lint_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${spirv-tools_SPIRV-Tools-lint_COMPILE_OPTIONS_C_DEBUG}>")
########### COMPONENT SPIRV-Tools-reduce VARIABLES ############################################

set(spirv-tools_SPIRV-Tools-reduce_INCLUDE_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_LIB_DIRS_DEBUG "${spirv-tools_PACKAGE_FOLDER_DEBUG}/lib")
set(spirv-tools_SPIRV-Tools-reduce_BIN_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_LIBRARY_TYPE_DEBUG STATIC)
set(spirv-tools_SPIRV-Tools-reduce_IS_HOST_WINDOWS_DEBUG 0)
set(spirv-tools_SPIRV-Tools-reduce_RES_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_OBJECTS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_COMPILE_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_COMPILE_OPTIONS_C_DEBUG "")
set(spirv-tools_SPIRV-Tools-reduce_COMPILE_OPTIONS_CXX_DEBUG "")
set(spirv-tools_SPIRV-Tools-reduce_LIBS_DEBUG SPIRV-Tools-reduce)
set(spirv-tools_SPIRV-Tools-reduce_SYSTEM_LIBS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_FRAMEWORK_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_FRAMEWORKS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_DEPENDENCIES_DEBUG SPIRV-Tools-static SPIRV-Tools-opt)
set(spirv-tools_SPIRV-Tools-reduce_SHARED_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_EXE_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-reduce_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(spirv-tools_SPIRV-Tools-reduce_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${spirv-tools_SPIRV-Tools-reduce_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${spirv-tools_SPIRV-Tools-reduce_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${spirv-tools_SPIRV-Tools-reduce_EXE_LINK_FLAGS_DEBUG}>
)
set(spirv-tools_SPIRV-Tools-reduce_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${spirv-tools_SPIRV-Tools-reduce_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${spirv-tools_SPIRV-Tools-reduce_COMPILE_OPTIONS_C_DEBUG}>")
########### COMPONENT SPIRV-Tools-link VARIABLES ############################################

set(spirv-tools_SPIRV-Tools-link_INCLUDE_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_LIB_DIRS_DEBUG "${spirv-tools_PACKAGE_FOLDER_DEBUG}/lib")
set(spirv-tools_SPIRV-Tools-link_BIN_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_LIBRARY_TYPE_DEBUG STATIC)
set(spirv-tools_SPIRV-Tools-link_IS_HOST_WINDOWS_DEBUG 0)
set(spirv-tools_SPIRV-Tools-link_RES_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_OBJECTS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_COMPILE_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_COMPILE_OPTIONS_C_DEBUG "")
set(spirv-tools_SPIRV-Tools-link_COMPILE_OPTIONS_CXX_DEBUG "")
set(spirv-tools_SPIRV-Tools-link_LIBS_DEBUG SPIRV-Tools-link)
set(spirv-tools_SPIRV-Tools-link_SYSTEM_LIBS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_FRAMEWORK_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_FRAMEWORKS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_DEPENDENCIES_DEBUG SPIRV-Tools-static SPIRV-Tools-opt)
set(spirv-tools_SPIRV-Tools-link_SHARED_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_EXE_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-link_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(spirv-tools_SPIRV-Tools-link_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${spirv-tools_SPIRV-Tools-link_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${spirv-tools_SPIRV-Tools-link_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${spirv-tools_SPIRV-Tools-link_EXE_LINK_FLAGS_DEBUG}>
)
set(spirv-tools_SPIRV-Tools-link_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${spirv-tools_SPIRV-Tools-link_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${spirv-tools_SPIRV-Tools-link_COMPILE_OPTIONS_C_DEBUG}>")
########### COMPONENT SPIRV-Tools-opt VARIABLES ############################################

set(spirv-tools_SPIRV-Tools-opt_INCLUDE_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_LIB_DIRS_DEBUG "${spirv-tools_PACKAGE_FOLDER_DEBUG}/lib")
set(spirv-tools_SPIRV-Tools-opt_BIN_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_LIBRARY_TYPE_DEBUG STATIC)
set(spirv-tools_SPIRV-Tools-opt_IS_HOST_WINDOWS_DEBUG 0)
set(spirv-tools_SPIRV-Tools-opt_RES_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_OBJECTS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_COMPILE_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_COMPILE_OPTIONS_C_DEBUG "")
set(spirv-tools_SPIRV-Tools-opt_COMPILE_OPTIONS_CXX_DEBUG "")
set(spirv-tools_SPIRV-Tools-opt_LIBS_DEBUG SPIRV-Tools-opt)
set(spirv-tools_SPIRV-Tools-opt_SYSTEM_LIBS_DEBUG m)
set(spirv-tools_SPIRV-Tools-opt_FRAMEWORK_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_FRAMEWORKS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_DEPENDENCIES_DEBUG SPIRV-Tools-static)
set(spirv-tools_SPIRV-Tools-opt_SHARED_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_EXE_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-opt_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(spirv-tools_SPIRV-Tools-opt_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${spirv-tools_SPIRV-Tools-opt_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${spirv-tools_SPIRV-Tools-opt_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${spirv-tools_SPIRV-Tools-opt_EXE_LINK_FLAGS_DEBUG}>
)
set(spirv-tools_SPIRV-Tools-opt_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${spirv-tools_SPIRV-Tools-opt_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${spirv-tools_SPIRV-Tools-opt_COMPILE_OPTIONS_C_DEBUG}>")
########### COMPONENT SPIRV-Tools-static VARIABLES ############################################

set(spirv-tools_SPIRV-Tools-static_INCLUDE_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_LIB_DIRS_DEBUG "${spirv-tools_PACKAGE_FOLDER_DEBUG}/lib")
set(spirv-tools_SPIRV-Tools-static_BIN_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_LIBRARY_TYPE_DEBUG STATIC)
set(spirv-tools_SPIRV-Tools-static_IS_HOST_WINDOWS_DEBUG 0)
set(spirv-tools_SPIRV-Tools-static_RES_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_OBJECTS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_COMPILE_DEFINITIONS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_COMPILE_OPTIONS_C_DEBUG "")
set(spirv-tools_SPIRV-Tools-static_COMPILE_OPTIONS_CXX_DEBUG "")
set(spirv-tools_SPIRV-Tools-static_LIBS_DEBUG SPIRV-Tools)
set(spirv-tools_SPIRV-Tools-static_SYSTEM_LIBS_DEBUG m rt stdc++)
set(spirv-tools_SPIRV-Tools-static_FRAMEWORK_DIRS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_FRAMEWORKS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_DEPENDENCIES_DEBUG )
set(spirv-tools_SPIRV-Tools-static_SHARED_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_EXE_LINK_FLAGS_DEBUG )
set(spirv-tools_SPIRV-Tools-static_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(spirv-tools_SPIRV-Tools-static_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${spirv-tools_SPIRV-Tools-static_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${spirv-tools_SPIRV-Tools-static_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${spirv-tools_SPIRV-Tools-static_EXE_LINK_FLAGS_DEBUG}>
)
set(spirv-tools_SPIRV-Tools-static_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${spirv-tools_SPIRV-Tools-static_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${spirv-tools_SPIRV-Tools-static_COMPILE_OPTIONS_C_DEBUG}>")