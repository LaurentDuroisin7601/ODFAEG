set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Entity)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Entity)

#file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${INCROOT}/*.hpp")

add_library(odfaeg-entity-mod STATIC)

target_sources(odfaeg-entity-mod
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS ${SRCROOT}
        FILES ${SRCROOT}/transformMatrix.ixx
            ${SRCROOT}/entity.ixx 
            ${SRCROOT}/impl.ixx  	       	
)

target_sources(odfaeg-entity-mod PRIVATE ${SRCROOT}/transformMatrix.cpp
            ${SRCROOT}/impl.cpp         	)
target_compile_features(odfaeg-entity-mod PUBLIC cxx_std_20)
target_include_directories(odfaeg-entity-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/entt)
target_link_libraries(odfaeg-entity-mod PUBLIC odfaeg-math-mod odfaeg-core-mod)

add_library(odfaeg-entity STATIC)

target_sources(odfaeg-entity
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS}
)

target_sources(odfaeg-entity PRIVATE ${SRC})
target_link_libraries(odfaeg-entity PRIVATE odfaeg-entity-mod)


install(
    TARGETS odfaeg-entity
    EXPORT odfaeg-targets
    FILE_SET HEADERS DESTINATION include
)
install(
        TARGETS odfaeg-entity-mod
        EXPORT odfaeg-targets
        FILE_SET cxx_modules DESTINATION share/odfaeg/modules/Entity
)
install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/odfaeg-entity-mod.dir/
    DESTINATION share/odfaeg/pcm/Entity
    FILES_MATCHING PATTERN "*.pcm" PATTERN "*.modmap"
)

install(
    FILES ${MODULES_INTERFACE}
    DESTINATION share/odfaeg/modules/Entity
)

