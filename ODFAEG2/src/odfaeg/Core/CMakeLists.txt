set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Core)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Core)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${INCROOT}/*.hpp")
file(GLOB_RECURSE MODULES_INTERFACE CONFIGURE_DEPENDS "${SRCROOT}/*.ixx")


# Target interne (modules)
add_library(odfaeg-core-mod STATIC)
target_include_directories(odfaeg-core-mod PRIVATE external/boost)
target_include_directories(odfaeg-core-mod PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_sources(odfaeg-core-mod
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS ${SRCROOT}
        FILES ${MODULES_INTERFACE}
)


target_compile_features(odfaeg-core-mod PUBLIC cxx_std_20)

# Target externe (SDK)
add_library(odfaeg-core STATIC)
target_sources(odfaeg-core
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS        
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS}
        				
)
target_include_directories(odfaeg-core-mod
    PUBLIC
        /usr/lib/llvm-18/include
)
target_sources(odfaeg-core PRIVATE ${SRC})

target_link_libraries(odfaeg-core PRIVATE odfaeg-core-mod)


target_link_libraries(odfaeg-core-mod PRIVATE /usr/lib/llvm-18/lib/libclang-18.so)
# Installation
install(
    TARGETS odfaeg-core
    EXPORT odfaeg-targets
    FILE_SET HEADERS DESTINATION include
)
install(
        TARGETS odfaeg-core-mod
        EXPORT odfaeg-targets
        FILE_SET cxx_modules DESTINATION share/odfaeg/modules/Core
)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/odfaeg-core-mod.dir/
    DESTINATION share/odfaeg/pcm/Core
    FILES_MATCHING PATTERN "*.pcm" PATTERN "*.modmap"
)




