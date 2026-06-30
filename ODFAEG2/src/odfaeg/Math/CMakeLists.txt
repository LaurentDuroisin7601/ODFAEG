set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Math)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Math)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${INCROOT}/*.hpp")
file(GLOB_RECURSE MODULES_INTERFACE CONFIGURE_DEPENDS "${SRCROOT}/*.ixx")
file(GLOB_RECURSE MODULES_IMPLEMENTATION CONFIGURE_DEPENDS "${SRCROOT}/*.cppm")

add_library(odfaeg-math-mod STATIC)

target_sources(odfaeg-math-mod
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS ${SRCROOT}
        FILES ${MODULES_INTERFACE}
)

target_sources(odfaeg-math-mod PRIVATE ${MODULES_IMPLEMENTATION})
target_compile_features(odfaeg-math-mod PUBLIC cxx_std_20)

target_link_libraries(odfaeg-math-mod PUBLIC odfaeg-core-mod)

add_library(odfaeg-math STATIC)

target_sources(odfaeg-math
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS}
)

target_sources(odfaeg-math PRIVATE ${SRC})
target_link_libraries(odfaeg-math PRIVATE odfaeg-math-mod)

install(
    TARGETS odfaeg-math
    EXPORT odfaeg-targets
    FILE_SET HEADERS DESTINATION include
)
install(
        TARGETS odfaeg-math-mod
        EXPORT odfaeg-targets
        FILE_SET cxx_modules DESTINATION share/odfaeg/modules/Math
)
install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/odfaeg-math-mod.dir/
    DESTINATION share/odfaeg/pcm/Math
    FILES_MATCHING PATTERN "*.pcm" PATTERN "*.modmap"
)


