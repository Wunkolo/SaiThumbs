cmake_minimum_required( VERSION 3.8.0 )
project( SaiThumbs )

### Standard
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

### Verbosity
set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS 1)

### Build artifacts
set( OUTPUT_DIRECTORY "bin" )

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}" )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}" )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}" )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}" )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}" )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}" )

### Flags
add_compile_options( /W3 )

# Force static linking of runtime
foreach(
	CurFlag
	CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
	# Replace all instances of /MD with /MT
	if(${CurFlag} MATCHES "/MD")
		string(REGEX REPLACE "/MD" "/MT" ${CurFlag} "${${CurFlag}}")
	endif(${CurFlag} MATCHES "/MD")
endforeach(CurFlag)

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

	source/SaiThumbs.def
	source/version.rc

	source/main.cpp
	source/SaiThumbProvider.cpp
	source/ClassFactory.cpp
	source/Globals.cpp
)
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	libsai Shlwapi.lib
)

configure_file(
	bin/install.bat
	${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_DIRECTORY}/install.bat
	COPYONLY
)

configure_file(
	bin/uninstall.bat
	${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_DIRECTORY}/uninstall.bat
	COPYONLY
)

### Tests
add_executable(
	comtest
	tests/comtest.cpp
)
