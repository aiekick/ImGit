set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

function(SetTargetProperties package type path)
  	if (TARGET ${package})
		set_target_properties(${package} PROPERTIES ${type} ${path})
	endif()
endfunction()

# we will fetch the library
# we are not maintening
include(cmake/NotMaintained/notmaintained.cmake)

## libe we are maintening
## order is important here
include(cmake/Maintained/maintained.cmake)
