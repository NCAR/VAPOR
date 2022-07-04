set(LIB osgl)

include(CMakeCopy.cmake)

set(BUILD_MESA OFF)
if (BUILD_PYTHON AND (UNIX AND NOT APPLE))
    set(BUILD_MESA ON)
endif()

set(SOFTWARE_ONLY OFF)
if (NOT SOFTWARE_ONLY)
    if (UNIX AND NOT APPLE)
        set(BUILD_EGL ON)
    endif()
endif()

if (NOT PROJECT_NAME)
	cmake_minimum_required (VERSION 3.10)
    project (${LIB})
	set (CMAKE_CXX_STANDARD 11)
    # find_package (OpenGL REQUIRED)
    # include_directories (${OPENGL_INCLUDE_DIRS})
    set(CMAKE_INSTALL_BINDIR bin)
    set(CMAKE_INSTALL_LIBDIR lib)
    set(CMAKE_INSTALL_INCLUDEDIR include/vapor)
    if(NOT CMAKE_BUILD_TYPE)
	    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    endif()
else ()
    set(CMAKE_INSTALL_BINDIR ${INSTALL_BIN_DIR})
    set(CMAKE_INSTALL_LIBDIR ${INSTALL_LIB_DIR})
    set(CMAKE_INSTALL_INCLUDEDIR ${INSTALL_INCLUDE_DIR})
endif ()

file (GLOB HDRS ./include/vapor/*.h)
file (GLOB SRCS ./*.cpp)
if (APPLE)
	file (GLOB OBJC_SRCS ./*.mm)
	list (APPEND SRCS ${OBJC_SRCS})
endif ()

list(FILTER SRCS EXCLUDE REGEX "test_[^/]*\\.cpp$")
list(FILTER SRCS EXCLUDE REGEX "ext_[^/]*\\.cpp$")

list(APPEND SRCS glad/src/egl.c glad/src/gl.c)
list(APPEND HDRS glad/include/glad/egl.h glad/include/glad/gl.h)
# file(COPY glad/include DESTINATION ${CMAKE_BINARY_DIR})

add_library (${LIB} SHARED ${SRCS} ${HDRS})
# if (NOT SOFTWARE_ONLY)
#     target_link_libraries (${LIB} ${OPENGL_LIBRARIES})
# endif()
target_link_libraries (${LIB} common)

if (APPLE)
	find_library (APPKIT AppKit)
    target_link_libraries (${LIB} ${APPKIT})
endif ()

if (UNIX AND NOT APPLE)
    target_link_libraries (${LIB} rt dl)
    if (BUILD_EGL)
        # target_link_libraries(${LIB} EGL)
        target_compile_definitions(${LIB} PRIVATE BUILD_EGL)
    endif ()
    if (BUILD_MESA)
        target_link_libraries (${LIB} OSMesa)
        target_compile_definitions(${LIB} PRIVATE BUILD_MESA)
    endif ()
endif ()

# if (CONDA_BUILD AND UNIX AND NOT APPLE)
# target_link_libraries (${LIB} rt)
# endif()


target_compile_options(${LIB} PRIVATE -Wno-deprecated-declarations)
# target_compile_options(${LIB} PRIVATE -Wno-conversion-null)
# target_compile_options(${LIB} PRIVATE -Wno-unknown-warning-option)

# file(COPY ${HDRS} DESTINATION ${CMAKE_BINARY_DIR}/include/vapor)
# foreach (HDR ${HDRS})
#     copy(${HDR} ${CMAKE_BINARY_DIR}/include/vapor) # Does not work with xcode...
# endforeach()
# target_include_directories (${LIB} PUBLIC ${CMAKE_BINARY_DIR}/include)

target_include_directories (${LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories (${LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)


install (
    TARGETS ${LIB}
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	COMPONENT Libraries
	)

install (
	FILES ${HDRS}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	COMPONENT Libraries
	)

install (
    DIRECTORY glad/include/glad glad/include/KHR glad/include/EGL
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	COMPONENT Libraries
    )

file (GLOB TESTS ./test_*.cpp)
foreach (TEST ${TESTS})
    get_filename_component(TEST_NAME "${TEST}" NAME_WE)
    set(TEST_TARGET "${LIB}_${TEST_NAME}")
    add_executable(${TEST_TARGET} "${TEST}")
    target_link_libraries(${TEST_TARGET} ${LIB})
    target_compile_options(${TEST_TARGET} PRIVATE -Wno-deprecated-declarations)
    install(TARGETS ${TEST_TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Libraries)
endforeach()
