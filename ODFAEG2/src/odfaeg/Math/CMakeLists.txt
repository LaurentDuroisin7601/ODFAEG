set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Math)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Math)

file(GLOB_RECURSE HEADERS_COMMON CONFIGURE_DEPENDS "${INCROOT}/*.hpp")
file(GLOB_RECURSE ODFAEG_INL "${SRCROOT}/.inl")
source_group("Inline Files" FILES ${ODFAEG_INL})
add_library(odfaeg-math STATIC)
target_sources(odfaeg-math
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS_COMMON}
)
if(ODFAEG_USE_MODULES)
    # Target interne (modules)
    add_library(odfaeg-math-mod STATIC)
    target_compile_features(odfaeg-math-mod PUBLIC cxx_std_20)
    file(GLOB_RECURSE MODULES_INTERFACE CONFIGURE_DEPENDS "${SRCROOT}/*.ixx")
    target_sources(odfaeg-math-mod
        PUBLIC
            FILE_SET cxx_modules TYPE CXX_MODULES
            BASE_DIRS ${SRCROOT}
            FILES ${MODULES_INTERFACE}
    )
    target_sources(odfaeg-math-mod PRIVATE ${HEADERS_COMMON} ${ODFAEG_INL})        
    target_link_libraries(odfaeg-math-mod PUBLIC odfaeg-core-mod)
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
else()
	file(GLOB_RECURSE HEADERS_SRC CONFIGURE_DEPENDS "${SRCROOT}/*.hpp")
	file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
    target_sources(odfaeg-math PRIVATE ${SRC} ${HEADERS_COMMON} ${HEADERS_SRC} ${ODFAEG_INL})    
    target_link_libraries(odfaeg-math PRIVATE odfaeg-core)
    # Installer la lib statique
	install(
		TARGETS odfaeg-math
		EXPORT odfaeg-targets
		ARCHIVE DESTINATION lib
		FILE_SET HEADERS DESTINATION include/odfaeg/Math
	)
	# Installer les headers publics
	install(
		DIRECTORY ${SRCROOT}/
		DESTINATION include/odfaeg/Math
		FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
	)
endif()






