set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Audio)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Audio)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
	"${SRCROOT}/*.cpp"
    	"${INCROOT}/*.hpp"
)
add_library(odfaeg-audio STATIC ${SRC})
target_include_directories(odfaeg-audio PUBLIC
    ${INCROOT}
)
target_link_libraries(odfaeg-audio PRIVATE odfaeg-math odfaeg-core)
find_package(SndFile REQUIRED)
find_package(OpenAL REQUIRED)
target_link_libraries(odfaeg-audio PRIVATE
    SndFile::sndfile
    OpenAL::OpenAL
)
install(TARGETS odfaeg-audio
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Installer les headers
install(DIRECTORY ${INCROOT}/
    DESTINATION include/odfaeg/Audio
    FILES_MATCHING PATTERN "*.hpp"
)