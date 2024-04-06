set(CTOOLS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ctools/include)
set(CTOOLS_LIBRARIES ctools)

set(USE_GL_VERSION_CHECKER OFF CACHE BOOL "" FORCE)
set(USE_CONFIG_SYSTEM ON CACHE BOOL "" FORCE)
set(USE_GLFW_CLIPBOARD ON CACHE BOOL "" FORCE)

include_directories(
	${GLFW_INCLUDE_DIR}
	${TINYXML2_INCLUDE_DIR}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ctools)

if(USE_SHARED_LIBS)
	SetTargetProperties(ctools FOLDER 3rdparty/aiekick/Shared)
else()
	SetTargetProperties(ctools FOLDER 3rdparty/aiekick/Static)
endif()

SetTargetProperties(ctools RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
SetTargetProperties(ctools RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
SetTargetProperties(ctools RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
SetTargetProperties(ctools LIBRARY_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
SetTargetProperties(ctools LIBRARY_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
SetTargetProperties(ctools LIBRARY_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
