include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/Modules/FindSFML.cmake)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Core)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Core)
set(SRC    
	${INCROOT}/archive.h
	${SRCROOT}/archive.cpp
	${INCROOT}/serialization.h
	${INCROOT}/entityFactory.hpp
	${INCROOT}/ecs.hpp
	${INCROOT}/erreur.h
	${INCROOT}/export.hpp
	${SRCROOT}/erreur.cpp	
	${INCROOT}/resourceManager.h		
	${INCROOT}/any.h
	${INCROOT}/resourceCache.h
	${INCROOT}/utilities.h
	${SRCROOT}/utilities.cpp	
	${INCROOT}/factory.h
	${INCROOT}/factory.cpp
	${INCROOT}/entitySystem.h
	${SRCROOT}/entitySystem.cpp
	${INCROOT}/timer.h
	${SRCROOT}/timer.cpp		
	${INCROOT}/state.h
	${SRCROOT}/state.cpp
	${INCROOT}/stateGroup.h
	${SRCROOT}/stateGroup.cpp
	${INCROOT}/stateStack.h
	${SRCROOT}/stateStack.cpp
	${INCROOT}/stateParameter.h
	${INCROOT}/stateParameterBase.h
	${INCROOT}/stateExecutor.h		
	${INCROOT}/archive.h	
	${INCROOT}/time.h
	${SRCROOT}/time.cpp
	${INCROOT}/clock.h
	${SRCROOT}/clock.cpp
	${INCROOT}/variant.h
	${INCROOT}/class.hpp
	${SRCROOT}/class.cpp
	${INCROOT}/constructor.hpp
	${SRCROOT}/constructor.cpp
	${INCROOT}/memberFunction.hpp
	${SRCROOT}/memberFunction.cpp
	${INCROOT}/runtimeCompiler.hpp
	${SRCROOT}/runtimeCompiler.cpp
	${INCROOT}/fastDelegate.h
	${INCROOT}/ecs.hpp
	${INCROOT}/utf.hpp
	${INCROOT}/string.hpp
	${SRCROOT}/string.cpp	
	${INCROOT}/inputStream.hpp
	${INCROOT}/fileInputStream.hpp
	${INCROOT}/memoryInputStream.hpp
	${SRCROOT}/fileInputStream.cpp
	${SRCROOT}/memoryInputStream.cpp	
	)	


link_directories(${CMAKE_LIBRARY_PATH})

#find_package(SFML 2.5 COMPONENTS network audio graphics window system REQUIRED)

include_directories(${CMAKE_INCLUDE_PATH})

sfgl_add_library(odfaeg-core
                 SOURCES ${SRC})

target_link_libraries (odfaeg-core)



