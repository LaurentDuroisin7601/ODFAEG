########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(imgui_COMPONENT_NAMES "")
if(DEFINED imgui_FIND_DEPENDENCY_NAMES)
  list(APPEND imgui_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES imgui_FIND_DEPENDENCY_NAMES)
else()
  set(imgui_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(imgui_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/imguicd4d864137763/p")
set(imgui_BUILD_MODULES_PATHS_RELEASE )


set(imgui_INCLUDE_DIRS_RELEASE "${imgui_PACKAGE_FOLDER_RELEASE}/include")
set(imgui_RES_DIRS_RELEASE )
set(imgui_DEFINITIONS_RELEASE )
set(imgui_SHARED_LINK_FLAGS_RELEASE )
set(imgui_EXE_LINK_FLAGS_RELEASE )
set(imgui_OBJECTS_RELEASE )
set(imgui_COMPILE_DEFINITIONS_RELEASE )
set(imgui_COMPILE_OPTIONS_C_RELEASE )
set(imgui_COMPILE_OPTIONS_CXX_RELEASE )
set(imgui_LIB_DIRS_RELEASE "${imgui_PACKAGE_FOLDER_RELEASE}/lib")
set(imgui_BIN_DIRS_RELEASE )
set(imgui_LIBRARY_TYPE_RELEASE STATIC)
set(imgui_IS_HOST_WINDOWS_RELEASE 0)
set(imgui_LIBS_RELEASE imgui)
set(imgui_SYSTEM_LIBS_RELEASE m)
set(imgui_FRAMEWORK_DIRS_RELEASE )
set(imgui_FRAMEWORKS_RELEASE )
set(imgui_BUILD_DIRS_RELEASE )
set(imgui_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(imgui_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${imgui_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${imgui_COMPILE_OPTIONS_C_RELEASE}>")
set(imgui_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${imgui_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${imgui_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${imgui_EXE_LINK_FLAGS_RELEASE}>")


set(imgui_COMPONENTS_RELEASE )