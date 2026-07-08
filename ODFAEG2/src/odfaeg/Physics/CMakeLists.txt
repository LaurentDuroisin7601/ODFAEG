set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Physics)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Physics)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${INCROOT}/*.hpp")

add_library(odfaeg-physics-mod STATIC)

target_sources(odfaeg-physics-mod
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS ${SRCROOT}
        FILES ${SRCROOT}/boundingBox.ixx
)
target_sources(odfaeg-physics-mod PRIVATE ${SRCROOT}/boundingBox.cpp)

target_compile_features(odfaeg-physics-mod PUBLIC cxx_std_20)

target_link_libraries(odfaeg-physics-mod PUBLIC odfaeg-entity-mod odfaeg-math-mod odfaeg-core-mod)

add_library(odfaeg-physics STATIC)

target_sources(odfaeg-physics
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS}
)
target_sources(odfaeg-physics PRIVATE ${SRC})
target_link_libraries(odfaeg-physics PRIVATE odfaeg-physics-mod)

install(
    TARGETS odfaeg-physics
    EXPORT odfaeg-targets
    FILE_SET HEADERS DESTINATION include
)
install(
        TARGETS odfaeg-physics-mod
        EXPORT odfaeg-targets
        FILE_SET cxx_modules DESTINATION share/odfaeg/modules/Physics
)
install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/odfaeg-physics-mod.dir/
    DESTINATION share/odfaeg/pcm/Physics
    FILES_MATCHING PATTERN "*.pcm" PATTERN "*.modmap"
)

install(
    FILES ${MODULES_INTERFACE}
    DESTINATION share/odfaeg/modules/Physics
)

