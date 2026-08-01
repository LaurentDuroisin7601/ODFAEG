set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Core)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Core)
file(GLOB_RECURSE HEADERS_COMMON CONFIGURE_DEPENDS "${INCROOT}/*.hpp")
file(GLOB_RECURSE ODFAEG_INL "${SRCROOT}/.inl")
source_group("Inline Files" FILES ${ODFAEG_INL})
# Target externe (SDK)
add_library(odfaeg-core STATIC)
target_sources(odfaeg-core
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS        
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS_COMMON}
        				
)
if(ODFAEG_USE_MODULES)
    	file(GLOB_RECURSE MODULES_INTERFACE CONFIGURE_DEPENDS "${SRCROOT}/*.ixx")
    	# Target interne (modules)
   	 add_library(odfaeg-core-mod STATIC)
    	target_compile_features(odfaeg-core-mod PUBLIC cxx_std_20)
   	 target_include_directories(odfaeg-core-mod PRIVATE external/boost)
	target_include_directories(odfaeg-core-mod PRIVATE ${PROJECT_SOURCE_DIR}/include)
	target_sources(odfaeg-core-mod
    		PUBLIC
       		FILE_SET cxx_modules TYPE CXX_MODULES
        	BASE_DIRS ${SRCROOT}
        	FILES ${MODULES_INTERFACE}
	)
	target_sources(odfaeg-core-mod PRIVATE ${HEADERS_COMMON} ${ODFAEG_INL})
	target_include_directories(odfaeg-core-mod PRIVATE "C:/Program Files/LLVM/include")
	target_link_libraries(odfaeg-core PRIVATE odfaeg-core-mod)
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
else()
	file(GLOB_RECURSE HEADERS_SRC CONFIGURE_DEPENDS "${SRCROOT}/*.hpp")
	file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
	target_include_directories(odfaeg-core PRIVATE external/boost)
	target_include_directories(odfaeg-core PRIVATE ${PROJECT_SOURCE_DIR}/include)
	target_include_directories(odfaeg-core PRIVATE "C:/Program Files/LLVM/include")
	target_sources(odfaeg-core PRIVATE ${SRC} ${HEADERS_COMMON} ${HEADERS_SRC} ${ODFAEG_INL})
		# Installer la lib statique
    	install(
			TARGETS odfaeg-core
			EXPORT odfaeg-targets
			ARCHIVE DESTINATION lib
			FILE_SET HEADERS DESTINATION include/odfaeg/Core
		)
    	# Installer les headers publics
    	install(
        	DIRECTORY ${SRCROOT}/
        	DESTINATION include/odfaeg/Core
        	FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
    	)
endif()





