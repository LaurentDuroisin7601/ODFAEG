set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Physics)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Physics)

file(GLOB_RECURSE HEADERS_COMMON CONFIGURE_DEPENDS "${INCROOT}/*.hpp")
file(GLOB_RECURSE ODFAEG_INL "${SRCROOT}/.inl")
source_group("Inline Files" FILES ${ODFAEG_INL})
add_library(odfaeg-physics STATIC)
target_sources(odfaeg-physics     
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS_COMMON}
)
if(ODFAEG_USE_MODULES)
    add_library(odfaeg-physics-mod STATIC)
    file(GLOB_RECURSE MODULES_INTERFACE CONFIGURE_DEPENDS "${SRCROOT}/*.ixx")
    target_compile_features(odfaeg-physics-mod PUBLIC cxx_std_20)
    target_sources(odfaeg-physics-mod
        PUBLIC
            FILE_SET cxx_modules TYPE CXX_MODULES
            BASE_DIRS ${SRCROOT}
            FILES ${MODULES_INTERFACE}
    )
    target_sources(odfaeg-physics-mod PRIVATE ${HEADERS_COMMON} ${ODFAEG_INL}) 
    target_link_libraries(odfaeg-physics-mod PUBLIC odfaeg-math-mod odfaeg-core-mod)

    
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
else()

    file(GLOB_RECURSE HEADERS_SRC CONFIGURE_DEPENDS "${SRCROOT}/*.hpp")
    file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
    target_sources(odfaeg-physics PRIVATE ${SRC} ${HEADERS_COMMON} ${HEADERS_SRC} ${ODFAEG_INL})
    target_link_libraries(odfaeg-physics PUBLIC odfaeg-math odfaeg-core)
    # Installer la lib statique
    install(
        TARGETS odfaeg-physics
        EXPORT odfaeg-targets
        ARCHIVE DESTINATION lib
        FILE_SET HEADERS DESTINATION include/odfaeg/Physics
    )
    # Installer les headers publics
    install(
        DIRECTORY ${SRCROOT}/
        DESTINATION include/odfaeg/Physics
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
    )
endif()

