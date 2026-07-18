set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Entity)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Entity)

#file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${INCROOT}/*.hpp")

add_library(odfaeg-entity-mod STATIC)

target_sources(odfaeg-entity-mod
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS ${SRCROOT}
        FILES
	    ${SRCROOT}/primitiveType.ixx
	    ${SRCROOT}/rect.ixx	
            ${SRCROOT}/assimpHelper.ixx		 
	    ${SRCROOT}/transformable.ixx          
            ${SRCROOT}/entity.ixx 
            ${SRCROOT}/impl.ixx
	    ${SRCROOT}/color.ixx
	    ${SRCROOT}/vertex.ixx
	    ${SRCROOT}/vertexArray.ixx
            ${SRCROOT}/gameObject.ixx
	    ${SRCROOT}/tile.ixx
	    ${SRCROOT}/model.ixx
            ${SRCROOT}/plane.ixx
	    ${SRCROOT}/cube.ixx
	    ${SRCROOT}/morphAnim.ixx
            ${SRCROOT}/bone.ixx
            ${SRCROOT}/animation.ixx
	    ${SRCROOT}/animator.ixx
            ${SRCROOT}/particle.ixx
	    ${SRCROOT}/emittors.ixx
	    ${SRCROOT}/particleSystem.ixx			
)

target_sources(odfaeg-entity-mod PRIVATE 
            ${SRCROOT}/impl.cpp   
            ${SRCROOT}/transformable.cpp           
            ${SRCROOT}/impl.cpp
	    ${SRCROOT}/color.cpp 
	    ${SRCROOT}/vertex.cpp
	    ${SRCROOT}/vertexArray.cpp
            ${SRCROOT}/gameObject.cpp
	    ${SRCROOT}/tile.cpp
	    ${SRCROOT}/model.cpp
            ${SRCROOT}/plane.cpp
	    ${SRCROOT}/cube.cpp
	    ${SRCROOT}/morphAnim.cpp
            ${SRCROOT}/bone.cpp
            ${SRCROOT}/animation.cpp
	    ${SRCROOT}/animator.cpp
	    ${SRCROOT}/particleSystem.cpp
)
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

