########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(openal-soft_COMPONENT_NAMES "")
if(DEFINED openal-soft_FIND_DEPENDENCY_NAMES)
  list(APPEND openal-soft_FIND_DEPENDENCY_NAMES ALSA)
  list(REMOVE_DUPLICATES openal-soft_FIND_DEPENDENCY_NAMES)
else()
  set(openal-soft_FIND_DEPENDENCY_NAMES ALSA)
endif()
set(ALSA_FIND_MODE "MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(openal-soft_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/opena2cd813ec58a8a/p")
set(openal-soft_BUILD_MODULES_PATHS_DEBUG "${openal-soft_PACKAGE_FOLDER_DEBUG}/lib/cmake/conan-official-openal-soft-variables.cmake")


set(openal-soft_INCLUDE_DIRS_DEBUG "${openal-soft_PACKAGE_FOLDER_DEBUG}/include"
			"${openal-soft_PACKAGE_FOLDER_DEBUG}/include/AL")
set(openal-soft_RES_DIRS_DEBUG )
set(openal-soft_DEFINITIONS_DEBUG "-DAL_LIBTYPE_STATIC")
set(openal-soft_SHARED_LINK_FLAGS_DEBUG )
set(openal-soft_EXE_LINK_FLAGS_DEBUG )
set(openal-soft_OBJECTS_DEBUG )
set(openal-soft_COMPILE_DEFINITIONS_DEBUG "AL_LIBTYPE_STATIC")
set(openal-soft_COMPILE_OPTIONS_C_DEBUG )
set(openal-soft_COMPILE_OPTIONS_CXX_DEBUG )
set(openal-soft_LIB_DIRS_DEBUG "${openal-soft_PACKAGE_FOLDER_DEBUG}/lib")
set(openal-soft_BIN_DIRS_DEBUG )
set(openal-soft_LIBRARY_TYPE_DEBUG STATIC)
set(openal-soft_IS_HOST_WINDOWS_DEBUG 0)
set(openal-soft_LIBS_DEBUG openal)
set(openal-soft_SYSTEM_LIBS_DEBUG dl m stdc++ atomic)
set(openal-soft_FRAMEWORK_DIRS_DEBUG )
set(openal-soft_FRAMEWORKS_DEBUG )
set(openal-soft_BUILD_DIRS_DEBUG )
set(openal-soft_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(openal-soft_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${openal-soft_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${openal-soft_COMPILE_OPTIONS_C_DEBUG}>")
set(openal-soft_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${openal-soft_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${openal-soft_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${openal-soft_EXE_LINK_FLAGS_DEBUG}>")


set(openal-soft_COMPONENTS_DEBUG )