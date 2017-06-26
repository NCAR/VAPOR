add_library (common SHARED common.cpp MyBase.cpp OptionParser.cpp EasyThreads.cpp CFuncs.cpp Version.cpp PVTime.cpp GetAppPath.cpp utils.cpp glutil.cpp)


if (APPLE)
	find_library (COREFOUNDATION CoreFoundation)
	target_link_libraries (common ${COREFOUNDATION})
endif()

if (NOT WIN32)
	add_definitions (-DENABLE_THREADS)
endif()

find_library (OPENGL OpenGL)
target_link_libraries (common GLEW ${OPENGL})

if (UNIX AND NOT APPLE)
	target_link_libraries (common GLU)
endif ()

target_compile_options(common PRIVATE "-Wno-deprecated-declarations")

# Not needed since vapor shares include directory
# target_include_directories (common PUBLIC ${VAPOR_SOURCE_DIR}/include)
