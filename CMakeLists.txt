cmake_minimum_required( VERSION 3.8.0 )
project( SaiThumbs )

set( LIBRARY_OUTPUT_DIRECTORY "build/" )
set( EXECUTABLE_OUTPUT_DIRECTORY "build/" )

### Standard
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

### Verbosity
set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS 1)

### Flags
add_compile_options( /W3 )

### Dependencies
add_subdirectory( external )

## libsai
include_directories(
	${LIBSAI_INCLUDE_DIRS}
)

### Target
include_directories(
	include
)
add_library(
	${PROJECT_NAME}
	SHARED
	source/main.cpp
	source/SaiThumbs.def
	source/SaiThumbProvider.cpp
	source/ClassFactory.cpp
	source/Globals.cpp
)
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	libsai Shlwapi.lib
)

### Tests
add_executable(
	comtest
	tests/comtest.cpp
)