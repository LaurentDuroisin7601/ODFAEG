set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Entity)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Entity)

#file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS_COMMON CONFIGURE_DEPENDS "${INCROOT}/*.hpp")
file(GLOB_RECURSE ODFAEG_INL "${SRCROOT}/.inl")
source_group("Inline Files" FILES ${ODFAEG_INL})
add_library(odfaeg-entity STATIC)
target_sources(odfaeg-entity
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS_COMMON}
)
if(ODFAEG_USE_MODULES)
    # Target interne (modules)
    add_library(odfaeg-entity-mod STATIC)
    target_compile_features(odfaeg-entity-mod PUBLIC cxx_std_20)
    file(GLOB_RECURSE MODULES_INTERFACE CONFIGURE_DEPENDS "${SRCROOT}/*.ixx")
    file(GLOB_RECURSE MODULES_IPMLEMENTATION CONFIGURE_DEPENDS "${SRCROOT}/*.cppm")
    target_sources(odfaeg-entity-mod
        PUBLIC
            FILE_SET cxx_modules TYPE CXX_MODULES
            BASE_DIRS ${SRCROOT}
            FILES ${MODULES_INTERFACE}
    )
    target_sources(odfaeg-entity-mod PRIVATE                     
                   ${MODULES_IMPLEMENTATION}
    )
    target_sources(odfaeg-entity-mod PRIVATE ${HEADERS_COMMON} ${ODFAEG_INL}) 
    target_include_directories(odfaeg-entity-mod PRIVATE ${PROJECT_SOURCE_DIR}/external)
    target_include_directories(odfaeg-entity-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/meshoptimizer/src)
    file(GLOB_RECURSE MESH_OPTIMIZER_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/external/meshoptimizer/src/*.cpp")
    add_library(meshoptimizer STATIC
        ${MESH_OPTIMIZER_SRC}
    )
    target_link_libraries(odfaeg-entity-mod PRIVATE meshoptimizer)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ASSIMP REQUIRED assimp)
    target_include_directories(odfaeg-entity-mod PRIVATE ${ASSIMP_INCLUDE_DIRS})
    target_link_directories(odfaeg-entity-mod PRIVATE ${ASSIMP_LIBRARY_DIRS})
    target_link_libraries(odfaeg-entity-mod PRIVATE ${ASSIMP_LIBRARIES})

    target_compile_features(odfaeg-entity-mod PUBLIC cxx_std_20)
    target_include_directories(odfaeg-entity-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/entt)
    target_link_libraries(odfaeg-entity-mod PUBLIC odfaeg-physics-mod odfaeg-math-mod odfaeg-core-mod)
    target_include_directories(odfaeg-entity-mod
            PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    )
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
else()
    file(GLOB_RECURSE HEADERS_SRC CONFIGURE_DEPENDS "${SRCROOT}/*.hpp")
	file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
    target_sources(odfaeg-entity PRIVATE ${SRC} ${HEADERS_COMMON} ${HEADERS_SRC} ${ODFAEG_INL})    
    target_link_libraries(odfaeg-entity PRIVATE odfaeg-core)
    target_include_directories(odfaeg-entity-mod PRIVATE ${PROJECT_SOURCE_DIR}/external)
    target_include_directories(odfaeg-entity-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/meshoptimizer/src)
    file(GLOB_RECURSE MESH_OPTIMIZER_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/external/meshoptimizer/src/*.cpp")
    add_library(meshoptimizer STATIC
        ${MESH_OPTIMIZER_SRC}
    )
    target_link_libraries(odfaeg-entity PRIVATE meshoptimizer)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ASSIMP REQUIRED assimp)
    target_include_directories(odfaeg-entity PRIVATE ${ASSIMP_INCLUDE_DIRS})
    target_link_directories(odfaeg-entity PRIVATE ${ASSIMP_LIBRARY_DIRS})
    target_link_libraries(odfaeg-entity PRIVATE ${ASSIMP_LIBRARIES})
    target_include_directories(odfaeg-entity
            PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    )
    # Installer la lib statique
	install(
		TARGETS odfaeg-entity
		EXPORT odfaeg-targets
		ARCHIVE DESTINATION lib
		FILE_SET HEADERS DESTINATION include/odfaeg/Entity
	)
	# Installer les headers publics
	install(
		DIRECTORY ${SRCROOT}/
		DESTINATION include/odfaeg/Entity
		FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
	)
endif()

