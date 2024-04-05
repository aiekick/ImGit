FetchContent_Declare(
	tinyxml2 
	GIT_REPOSITORY	https://github.com/leethomason/tinyxml2.git
	GIT_TAG			10.0.0
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/tinyxml2
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(tinyxml2 )
if(NOT tinyxml2_POPULATED)
	FetchContent_Populate(tinyxml2 )

	if(USE_SHARED_LIBS)
		set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
		set(LLVM_USE_CRT_DEBUG MDd CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_MINSIZEREL MD CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELEASE MD CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELWITHDEBINFO MD CACHE STRING "" FORCE)
	else()
		set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
		set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
	endif()
	
	if(NOT CMAKE_DEBUG_POSTFIX)
	  set(CMAKE_DEBUG_POSTFIX _debug)
	endif()
	if(NOT CMAKE_RELEASE_POSTFIX)
	  set(CMAKE_RELEASE_POSTFIX)
	endif()
	if(NOT CMAKE_MINSIZEREL_POSTFIX)
	  set(CMAKE_MINSIZEREL_POSTFIX _minsizerel)
	endif()
	if(NOT CMAKE_RELWITHDEBINFO_POSTFIX)
	  set(CMAKE_RELWITHDEBINFO_POSTFIX _reldeb)
	endif()

	if (BUILD_SHARED_LIBS)
		set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)
		add_library(tinyxml2 
			${tinyxml2_SOURCE_DIR}/tinyxml2.cpp 
			${tinyxml2_SOURCE_DIR}/tinyxml2.h
		)
		set_target_properties(tinyxml2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
		set_target_properties(tinyxml2 PROPERTIES DEFINE_SYMBOL "TINYXML2_EXPORT")
	else()
		set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
		add_library(tinyxml2 STATIC 
			${tinyxml2_SOURCE_DIR}/tinyxml2.cpp 
			${tinyxml2_SOURCE_DIR}/tinyxml2.h
		)
	endif()

	if(USE_SHARED_LIBS)
		set_target_properties(tinyxml2 PROPERTIES FOLDER 3rdparty/Shared)
		set_target_properties(tinyxml2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
		set_target_properties(tinyxml2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
		set_target_properties(tinyxml2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}")
		set_target_properties(tinyxml2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}")
	else()
		set_target_properties(tinyxml2 PROPERTIES FOLDER 3rdparty/Static)
	endif()
	
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(tinyxml2 PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
	endif()
	
	set(TINYXML2_INCLUDE_DIR ${tinyxml2_SOURCE_DIR})
	set(TINYXML2_LIBRARIES tinyxml2)
endif()
