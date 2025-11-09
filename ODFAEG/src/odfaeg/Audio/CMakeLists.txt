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
	${SRCROOT}/soundFile.hpp
	${SRCROOT}/soundFile.cpp
	${INCROOT}/soundBuffer.hpp
	${SRCROOT}/soundBuffer.cpp
	${INCROOT}/soundStream.hpp
	${SRCROOT}/soundStream.cpp
	${INCROOT}/listener.hpp
	${SRCROOT}/listener.cpp
	${INCROOT}/soundSource.hpp
	${SRCROOT}/soundSource.cpp
	${INCROOT}/alResource.hpp
	${SRCROOT}/alResource.cpp
	${INCROOT}/sound.hpp
	${SRCROOT}/sound.cpp
	${INCROOT}/inputSoundFile.hpp
	${SRCROOT}/inputSoundFile.cpp
	${SRCROOT}/audioDevice.hpp
	${SRCROOT}/audioDevice.cpp
	${SRCROOT}/alCheck.hpp
	${SRCROOT}/alCheck.cpp
	${INCROOT}/soundFileFactory.hpp
	${INCROOT}/soundFileFactory.inl
	${SRCROOT}/soundFileFactory.cpp
	${SRCROOT}/aLCheck.cpp
	${SRCROOT}/aLCheck.hpp
	${INCROOT}/soundFileReader.hpp
	${SRCROOT}/soundFileReaderFlac.hpp
	${SRCROOT}/soundFileReaderFlac.cpp
	${SRCROOT}/soundFileReaderOgg.hpp
	${SRCROOT}/soundFileReaderOgg.cpp
	${SRCROOT}/soundFileReaderWav.hpp
	${SRCROOT}/soundFileReaderWav.cpp
	)
	
include_directories(${CMAKE_INCLUDE_PATH})
link_directories (${CMAKE_LIBRARY_PATH})
#
#find_package (SFML 2 COMPONENTS window system graphics audio REQUIRED)
sfgl_add_library(odfaeg-audio
				 SOURCES ${SRC})
if (COMPILER_MSVC)
	target_link_libraries (odfaeg-audio PUBLIC odfaeg-core sndfile)
else()
	target_link_libraries (odfaeg-audio PUBLIC odfaeg-core libsndfile)
endif()