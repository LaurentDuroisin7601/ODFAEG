# Conan automatically generated toolchain file
# DO NOT EDIT MANUALLY, it will be overwritten

# Avoid including toolchain file several times (bad if appending to variables like
#   CMAKE_CXX_FLAGS. See https://github.com/android/ndk/issues/323
include_guard()
message(STATUS "Using Conan toolchain: ${CMAKE_CURRENT_LIST_FILE}")
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeToolchain' generator only works with CMake >= 3.15")
endif()

########## 'user_toolchain' block #############
# Include one or more CMake user toolchain from tools.cmake.cmaketoolchain:user_toolchain



########## 'generic_system' block #############
# Definition of system, platform and toolset





########## 'compilers' block #############



########## 'arch_flags' block #############
# Define C++ flags, C flags and linker flags from 'settings.arch'
message(STATUS "Conan toolchain: Defining architecture flag: -m64")
string(APPEND CONAN_CXX_FLAGS " -m64")
string(APPEND CONAN_C_FLAGS " -m64")
string(APPEND CONAN_SHARED_LINKER_FLAGS " -m64")
string(APPEND CONAN_EXE_LINKER_FLAGS " -m64")


########## 'rpath_link_flags' block #############
# Pass -rpath-link pointing to all directories with runtime libraries


########## 'libcxx' block #############
# Definition of libcxx from 'compiler.libcxx' setting, defining the
# right CXX_FLAGS for that libcxx

message(STATUS "Conan toolchain: Adding glibcxx compile definition: _GLIBCXX_USE_CXX11_ABI=0")
add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)


########## 'cppstd' block #############
# Define the C++ and C standards from 'compiler.cppstd' and 'compiler.cstd'

function(conan_modify_std_watch variable access value current_list_file stack)
    set(conan_watched_std_variable "20")
    if (${variable} STREQUAL "CMAKE_C_STANDARD")
        set(conan_watched_std_variable "")
    endif()
    if ("${access}" STREQUAL "MODIFIED_ACCESS" AND NOT "${value}" STREQUAL "${conan_watched_std_variable}")
        message(STATUS "Warning: Standard ${variable} value defined in conan_toolchain.cmake to ${conan_watched_std_variable} has been modified to ${value} by ${current_list_file}")
    endif()
    unset(conan_watched_std_variable)
endfunction()

message(STATUS "Conan toolchain: C++ Standard 20 with extensions OFF")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
variable_watch(CMAKE_CXX_STANDARD conan_modify_std_watch)


########## 'extra_flags' block #############
# Include extra C++, C and linker flags from configuration tools.build:<type>flags
# and from CMakeToolchain.extra_<type>_flags

# Conan conf flags start: 
# Conan conf flags end


########## 'cmake_flags_init' block #############
# Define CMAKE_<XXX>_FLAGS from CONAN_<XXX>_FLAGS

foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER ${config} config)
    if(DEFINED CONAN_CXX_FLAGS_${config})
      string(APPEND CMAKE_CXX_FLAGS_${config}_INIT " ${CONAN_CXX_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_C_FLAGS_${config})
      string(APPEND CMAKE_C_FLAGS_${config}_INIT " ${CONAN_C_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_SHARED_LINKER_FLAGS_${config})
      string(APPEND CMAKE_SHARED_LINKER_FLAGS_${config}_INIT " ${CONAN_SHARED_LINKER_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_EXE_LINKER_FLAGS_${config})
      string(APPEND CMAKE_EXE_LINKER_FLAGS_${config}_INIT " ${CONAN_EXE_LINKER_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_RC_FLAGS_${config})
      string(APPEND CMAKE_RC_FLAGS_${config}_INIT " ${CONAN_RC_FLAGS_${config}}")
    endif()
endforeach()

if(DEFINED CONAN_CXX_FLAGS)
  string(APPEND CMAKE_CXX_FLAGS_INIT " ${CONAN_CXX_FLAGS}")
endif()
if(DEFINED CONAN_C_FLAGS)
  string(APPEND CMAKE_C_FLAGS_INIT " ${CONAN_C_FLAGS}")
endif()
if(DEFINED CONAN_SHARED_LINKER_FLAGS)
  string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " ${CONAN_SHARED_LINKER_FLAGS}")
endif()
if(DEFINED CONAN_EXE_LINKER_FLAGS)
  string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${CONAN_EXE_LINKER_FLAGS}")
endif()
if(DEFINED CONAN_RC_FLAGS)
  string(APPEND CMAKE_RC_FLAGS_INIT " ${CONAN_RC_FLAGS}")
endif()
if(DEFINED CONAN_OBJCXX_FLAGS)
  string(APPEND CMAKE_OBJCXX_FLAGS_INIT " ${CONAN_OBJCXX_FLAGS}")
endif()
if(DEFINED CONAN_OBJC_FLAGS)
  string(APPEND CMAKE_OBJC_FLAGS_INIT " ${CONAN_OBJC_FLAGS}")
endif()


########## 'extra_variables' block #############
# Definition of extra CMake variables from tools.cmake.cmaketoolchain:extra_variables



########## 'try_compile' block #############
# Blocks after this one will not be added when running CMake try/checks
get_property( _CMAKE_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE )
if(_CMAKE_IN_TRY_COMPILE)
    message(STATUS "Running toolchain IN_TRY_COMPILE")
    return()
endif()


########## 'find_paths' block #############
# Define paths to find packages, programs, libraries, etc.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/conan_cmakedeps_paths.cmake")
  message(STATUS "Conan toolchain: Including CMakeConfigDeps generated conan_cmakedeps_paths.cmake")
  include("${CMAKE_CURRENT_LIST_DIR}/conan_cmakedeps_paths.cmake")
else()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Definition of CMAKE_MODULE_PATH
list(PREPEND CMAKE_MODULE_PATH "/home/laurent/.conan2/p/b/openscc873457aaabf/p/lib/cmake")
# the generators folder (where conan generates files, like this toolchain)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Definition of CMAKE_PREFIX_PATH, CMAKE_XXXXX_PATH
# The explicitly defined "builddirs" of "host" context dependencies must be in PREFIX_PATH
list(PREPEND CMAKE_PREFIX_PATH "/home/laurent/.conan2/p/b/openscc873457aaabf/p/lib/cmake")
# The Conan local "generators" folder, where this toolchain is saved.
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR} )
list(PREPEND CMAKE_LIBRARY_PATH "/home/laurent/.conan2/p/b/assim1430cbbc53906/p/lib" "/home/laurent/.conan2/p/b/miniz6da750ec9a031/p/lib" "/home/laurent/.conan2/p/b/pugixc4add3f232626/p/lib" "lib" "/home/laurent/.conan2/p/b/kuba-87c04591aee84/p/lib" "/home/laurent/.conan2/p/b/poly25edfc427eed4d/p/lib" "lib" "/home/laurent/.conan2/p/b/dracoda068cd4a19f1/p/lib" "/home/laurent/.conan2/p/b/clippded9a3338d852/p/lib" "/home/laurent/.conan2/p/b/opend7e5f1fb4653cf/p/lib" "/home/laurent/.conan2/p/b/freetc5315ef469cc1/p/lib" "/home/laurent/.conan2/p/b/libpn7243b286b7924/p/lib" "/home/laurent/.conan2/p/b/brotle2cfd32feebb1/p/lib" "/home/laurent/.conan2/p/b/opena356eb11183869/p/lib" "/home/laurent/.conan2/p/b/libal3843900a0a299/p/lib" "/home/laurent/.conan2/p/b/libsnfc3277acb96c2/p/lib" "/home/laurent/.conan2/p/b/vorbi34281f4545253/p/lib" "/home/laurent/.conan2/p/b/flac5c22bc795a1d5/p/lib" "/home/laurent/.conan2/p/b/oggdc0bd6cb753eb/p/lib" "/home/laurent/.conan2/p/b/opus1e7c15cfa7c2f/p/lib" "/home/laurent/.conan2/p/b/mpg124199c929f1762/p/lib" "/home/laurent/.conan2/p/b/libmp3e3ea9e1b7754/p/lib" "/home/laurent/.conan2/p/b/openscc873457aaabf/p/lib" "/home/laurent/.conan2/p/b/boostd2dc739ac35ce/p/lib" "/home/laurent/.conan2/p/b/zlibf7662cd68a526/p/lib" "/home/laurent/.conan2/p/b/bzip2cd7fbe3a3d690/p/lib" "/home/laurent/.conan2/p/b/libba5e0e2b9c8ea1e/p/lib" "/home/laurent/.conan2/p/b/shade666a6e1056262/p/lib" "/home/laurent/.conan2/p/b/glslacdd7987ba73f6/p/lib" "/home/laurent/.conan2/p/b/spirvd341e24e50396/p/lib" "lib" "/home/laurent/.conan2/p/b/imguicd4d864137763/p/lib" "/home/laurent/.conan2/p/b/glfw61a8d675833bd/p/lib")
list(PREPEND CMAKE_INCLUDE_PATH "/home/laurent/.conan2/p/b/assim1430cbbc53906/p/include" "/home/laurent/.conan2/p/b/miniz6da750ec9a031/p/include" "/home/laurent/.conan2/p/b/miniz6da750ec9a031/p/include/minizip" "/home/laurent/.conan2/p/b/pugixc4add3f232626/p/include" "include" "/home/laurent/.conan2/p/b/kuba-87c04591aee84/p/include" "/home/laurent/.conan2/p/b/poly25edfc427eed4d/p/include" "include" "/home/laurent/.conan2/p/b/dracoda068cd4a19f1/p/include" "/home/laurent/.conan2/p/b/clippded9a3338d852/p/include" "/home/laurent/.conan2/p/b/opend7e5f1fb4653cf/p/include" "/home/laurent/.conan2/p/b/freetc5315ef469cc1/p/include" "/home/laurent/.conan2/p/b/freetc5315ef469cc1/p/include/freetype2" "/home/laurent/.conan2/p/b/libpn7243b286b7924/p/include" "/home/laurent/.conan2/p/b/brotle2cfd32feebb1/p/include" "/home/laurent/.conan2/p/b/opena356eb11183869/p/include" "/home/laurent/.conan2/p/b/opena356eb11183869/p/include/AL" "/home/laurent/.conan2/p/b/libal3843900a0a299/p/include" "/home/laurent/.conan2/p/b/libsnfc3277acb96c2/p/include" "/home/laurent/.conan2/p/b/vorbi34281f4545253/p/include" "/home/laurent/.conan2/p/b/flac5c22bc795a1d5/p/include" "/home/laurent/.conan2/p/b/oggdc0bd6cb753eb/p/include" "/home/laurent/.conan2/p/b/opus1e7c15cfa7c2f/p/include" "/home/laurent/.conan2/p/b/opus1e7c15cfa7c2f/p/include/opus" "/home/laurent/.conan2/p/b/mpg124199c929f1762/p/include" "/home/laurent/.conan2/p/b/libmp3e3ea9e1b7754/p/include" "/home/laurent/.conan2/p/b/openscc873457aaabf/p/include" "/home/laurent/.conan2/p/enttb1832d5c14fb1/p/include" "/home/laurent/.conan2/p/b/boostd2dc739ac35ce/p/include" "/home/laurent/.conan2/p/b/zlibf7662cd68a526/p/include" "/home/laurent/.conan2/p/b/bzip2cd7fbe3a3d690/p/include" "/home/laurent/.conan2/p/b/libba5e0e2b9c8ea1e/p/include" "/home/laurent/.conan2/p/b/shade666a6e1056262/p/include" "/home/laurent/.conan2/p/b/glslacdd7987ba73f6/p/include" "/home/laurent/.conan2/p/b/spirvd341e24e50396/p/include" "include" "/home/laurent/.conan2/p/stb6342cecb318f5/p/include" "/home/laurent/.conan2/p/b/imguicd4d864137763/p/include" "/home/laurent/.conan2/p/b/glfw61a8d675833bd/p/include")
set(CONAN_RUNTIME_LIB_DIRS "/home/laurent/.conan2/p/b/assim1430cbbc53906/p/lib" "/home/laurent/.conan2/p/b/miniz6da750ec9a031/p/lib" "/home/laurent/.conan2/p/b/pugixc4add3f232626/p/lib" "lib" "/home/laurent/.conan2/p/b/kuba-87c04591aee84/p/lib" "/home/laurent/.conan2/p/b/poly25edfc427eed4d/p/lib" "lib" "/home/laurent/.conan2/p/b/dracoda068cd4a19f1/p/lib" "/home/laurent/.conan2/p/b/clippded9a3338d852/p/lib" "/home/laurent/.conan2/p/b/opend7e5f1fb4653cf/p/lib" "/home/laurent/.conan2/p/b/freetc5315ef469cc1/p/lib" "/home/laurent/.conan2/p/b/libpn7243b286b7924/p/lib" "/home/laurent/.conan2/p/b/brotle2cfd32feebb1/p/lib" "/home/laurent/.conan2/p/b/opena356eb11183869/p/lib" "/home/laurent/.conan2/p/b/libal3843900a0a299/p/lib" "/home/laurent/.conan2/p/b/libsnfc3277acb96c2/p/lib" "/home/laurent/.conan2/p/b/vorbi34281f4545253/p/lib" "/home/laurent/.conan2/p/b/flac5c22bc795a1d5/p/lib" "/home/laurent/.conan2/p/b/oggdc0bd6cb753eb/p/lib" "/home/laurent/.conan2/p/b/opus1e7c15cfa7c2f/p/lib" "/home/laurent/.conan2/p/b/mpg124199c929f1762/p/lib" "/home/laurent/.conan2/p/b/libmp3e3ea9e1b7754/p/lib" "/home/laurent/.conan2/p/b/openscc873457aaabf/p/lib" "/home/laurent/.conan2/p/b/boostd2dc739ac35ce/p/lib" "/home/laurent/.conan2/p/b/zlibf7662cd68a526/p/lib" "/home/laurent/.conan2/p/b/bzip2cd7fbe3a3d690/p/lib" "/home/laurent/.conan2/p/b/libba5e0e2b9c8ea1e/p/lib" "/home/laurent/.conan2/p/b/shade666a6e1056262/p/lib" "/home/laurent/.conan2/p/b/glslacdd7987ba73f6/p/lib" "/home/laurent/.conan2/p/b/spirvd341e24e50396/p/lib" "lib" "/home/laurent/.conan2/p/b/imguicd4d864137763/p/lib" "/home/laurent/.conan2/p/b/glfw61a8d675833bd/p/lib" )

endif()


########## 'pkg_config' block #############
# Define pkg-config from 'tools.gnu:pkg_config' executable and paths

if (DEFINED ENV{PKG_CONFIG_PATH})
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}:$ENV{PKG_CONFIG_PATH}")
else()
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}:")
endif()


########## 'rpath' block #############
# Defining CMAKE_SKIP_RPATH



########## 'output_dirs' block #############
# Definition of CMAKE_INSTALL_XXX folders

# Ensure export(PACKAGE) honors CMAKE_EXPORT_PACKAGE_REGISTRY even if the
# project sets cmake_minimum_required() lower than 3.15.
cmake_policy(SET CMP0090 NEW)
if(NOT DEFINED CMAKE_EXPORT_PACKAGE_REGISTRY)
    set(CMAKE_EXPORT_PACKAGE_REGISTRY OFF)
endif()

set(CMAKE_INSTALL_BINDIR "bin")
set(CMAKE_INSTALL_SBINDIR "bin")
set(CMAKE_INSTALL_LIBEXECDIR "bin")
set(CMAKE_INSTALL_LIBDIR "lib")
set(CMAKE_INSTALL_INCLUDEDIR "include")
set(CMAKE_INSTALL_OLDINCLUDEDIR "include")


########## 'variables' block #############
# Definition of CMake variables from CMakeToolchain.variables values

# Variables
# Variables  per configuration



########## 'preprocessor' block #############
# Preprocessor definitions from CMakeToolchain.preprocessor_definitions values

# Preprocessor definitions per configuration



if(CMAKE_POLICY_DEFAULT_CMP0091)  # Avoid unused and not-initialized warnings
endif()
