FetchContent_Declare(
	libgit2
	GIT_REPOSITORY	https://github.com/libgit2/libgit2.git
	GIT_TAG			v1.8.0
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/libgit2
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(libgit2)
if(NOT libgit2_POPULATED)
	FetchContent_Populate(libgit2)
	
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
	
	##EXCLUDE_FROM_ALL reject install for this target
	add_subdirectory(${libgit2_SOURCE_DIR} EXCLUDE_FROM_ALL)	

	if(USE_SHARED_LIBS)
		set_target_properties(libgit2package PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
		set_target_properties(libgit2package PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
		set_target_properties(libgit2package PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}")
		set_target_properties(libgit2package PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}")
		set_target_properties(libgit2package PROPERTIES FOLDER 3rdparty/Shared/git2)
		set_target_properties(http-parser PROPERTIES FOLDER 3rdparty/Shared/git2)		
		set_target_properties(libgit2 PROPERTIES FOLDER 3rdparty/Shared/git2)
		set_target_properties(xdiff PROPERTIES FOLDER 3rdparty/Shared/git2)		
		set_target_properties(pcre PROPERTIES FOLDER 3rdparty/Shared/git2)	
		set_target_properties(util PROPERTIES FOLDER 3rdparty/Shared/git2)	
		set_target_properties(zlib PROPERTIES FOLDER 3rdparty/Shared/git2)	
	else()
		set_target_properties(libgit2package PROPERTIES FOLDER 3rdparty/Static/git2)
		set_target_properties(http-parser PROPERTIES FOLDER 3rdparty/Static/git2)			
		set_target_properties(libgit2 PROPERTIES FOLDER 3rdparty/Static/git2)
		set_target_properties(xdiff PROPERTIES FOLDER 3rdparty/Static/git2)		
		set_target_properties(pcre PROPERTIES FOLDER 3rdparty/Static/git2)	
		set_target_properties(util PROPERTIES FOLDER 3rdparty/Static/git2)	
		set_target_properties(zlib PROPERTIES FOLDER 3rdparty/Static/git2)	
	endif()
	
	set(LIB_GIT2_LIBRARIES libgit2package)
	set(LIB_GIT2_INCLUDE_DIR ${libgit2_SOURCE_DIR}/include)
	include_directories(${LIB_GIT2_INCLUDE_DIR})
endif()
