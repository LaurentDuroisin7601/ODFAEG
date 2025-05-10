
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Main)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Main)

# sources
if(SFML_OS_WINDOWS)
    set(SRC ${SRC} ${SRCROOT}/MainWin32.cpp)
elseif(SFML_OS_IOS)
    set(SRC ${SRC} ${SRCROOT}/MainiOS.mm)
else()
    return()
endif()

# define the sfml-main target
sfgl_add_library(odfaeg-main STATIC SOURCES ${SRC})

# overwrite sfml-main suffix for backward compatibility with FindSFML.cmake
set_target_properties(sfml-main PROPERTIES
                      DEBUG_POSTFIX -d
                      RELEASE_POSTFIX ""
                      MINSIZEREL_POSTFIX ""
                      RELWITHDEBINFO_POSTFIX "")

# because of a current limitation on Android (which prevents one library
# from depending on shared libraries), we need a boostrap activity which
# will load our shared libraries manually

