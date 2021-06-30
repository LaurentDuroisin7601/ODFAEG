# include the ODFAEG specific macros
include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)

set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Audio)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Audio)
set(SRC	
	${INCROOT}/export.hpp
	${INCROOT}/player.h
	${SRCROOT}/player.cpp
	${INCROOT}/stream.h
	${SRCROOT}/stream.cpp
	${SRCROOT}/SoundFile.hpp
	${SRCROOT}/SoundFile.cpp)
	
include_directories(${CMAKE_INCLUDE_PATH})
link_directories (${CMAKE_LIBRARY_PATH})
find_package (SFML 2 REQUIRED)
sfgl_add_library(odfaeg-audio
				 SOURCES ${SRC}
				 DEPENDS odfaeg-core)
target_link_libraries (odfaeg-audio ${SFML_LIBRARIES})

