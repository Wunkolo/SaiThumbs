cmake_minimum_required( VERSION 3.8.0 )
project( SaiThumbs )

### Standard
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

### Verbosity
set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

### Build artifacts
set( OUTPUT_DIRECTORY "bin" )

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}" )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}" )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}" )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}" )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}" )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}" )

### Flags
add_compile_options(
	/MP # Parallel builds 
	/permissive- # Stricter C++ conformance

	# Warnings
	/W3

	# Consider these warnings as errors
	/we4018 # 'expression': signed/unsigned mismatch
	/we4062 # Enumerator 'identifier' in a switch of enum 'enumeration' is not handled
	/we4101 # 'identifier': unreferenced local variable
	/we4265 # 'class': class has virtual functions, but destructor is not virtual
	/we4305 # 'context': truncation from 'type1' to 'type2'
	/we4388 # 'expression': signed/unsigned mismatch
	/we4389 # 'operator': signed/unsigned mismatch

	/we4456 # Declaration of 'identifier' hides previous local declaration
	/we4457 # Declaration of 'identifier' hides function parameter
	/we4458 # Declaration of 'identifier' hides class member
	/we4459 # Declaration of 'identifier' hides global declaration

	/we4505 # 'function': unreferenced local function has been removed
	/we4547 # 'operator': operator before comma has no effect; expected operator with side-effect
	/we4549 # 'operator1': operator before comma has no effect; did you intend 'operator2'?
	/we4555 # Expression has no effect; expected expression with side-effect
	/we4715 # 'function': not all control paths return a value
	/we4834 # Discarding return value of function with 'nodiscard' attribute
	/we5038 # data member 'member1' will be initialized after data member 'member2'
	/we5245 # 'function': unreferenced function with internal linkage has been removed

)

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
add_subdirectory( external/libsai )

### Target
include_directories(
	include
)
add_library(
	SaiThumbs
	SHARED

	source/SaiThumbs.def
	source/version.rc

	source/stb_image_resize2.c

	source/main.cpp
	source/Sai1ThumbProvider.cpp
	source/Sai2ThumbProvider.cpp
	source/Globals.cpp
)
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	sai Shlwapi.lib
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
