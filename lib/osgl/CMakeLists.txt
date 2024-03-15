set(LIB osgl)

if (BUILD_PYTHON AND (UNIX AND NOT APPLE))
    set(BUILD_MESA_DEFAULT ON)
endif()

option (BUILD_MESA "Build with OSMesa" ${BUILD_MESA_DEFAULT})
option (BUILD_OSGL_TESTS "Build tests" ON)

option (SOFTWARE_ONLY "Disable EGL" OFF)
if (NOT SOFTWARE_ONLY)
    if (UNIX AND NOT APPLE)
        set(BUILD_EGL ON)
    endif()
endif()

if (NOT PROJECT_NAME)
	cmake_minimum_required (VERSION 3.10)
    project (${LIB})
	set (CMAKE_CXX_STANDARD 11)
    set(CMAKE_INSTALL_BINDIR bin)
    set(CMAKE_INSTALL_LIBDIR lib)
    set(CMAKE_INSTALL_INCLUDEDIR include)
    if(NOT CMAKE_BUILD_TYPE)
	    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    endif()
else ()
    set(CMAKE_INSTALL_BINDIR ${INSTALL_BIN_DIR})
    set(CMAKE_INSTALL_LIBDIR ${INSTALL_LIB_DIR})
    set(CMAKE_INSTALL_INCLUDEDIR ${INSTALL_INCLUDE_DIR})
endif ()

file (GLOB HDRS ./include/osgl/*.h)
file (GLOB SRCS ./src/*.cpp)
if (APPLE)
	file (GLOB OBJC_SRCS ./src/*.mm)
	list (APPEND SRCS ${OBJC_SRCS})
endif ()

list(APPEND SRCS glad/src/egl.c glad/src/gl.c)
# list(APPEND HDRS glad/include/glad/egl.h glad/include/glad/gl.h)
if (WIN32)
    list(APPEND SRCS glad/src/wgl.c)
    # list(APPEND HDRS glad/include/glad/wgl.h)
endif()

add_library (${LIB} SHARED ${SRCS} ${HDRS})

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

if (NOT WIN32)
    target_compile_options(${LIB} PRIVATE -Wno-deprecated-declarations)
endif()

target_include_directories (${LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories (${LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

if (WIN32)
    target_compile_definitions(${LIB} PUBLIC GLAD_API_CALL_EXPORT=1)
    target_compile_definitions(${LIB} PRIVATE GLAD_API_CALL_EXPORT_BUILD=1)
    target_compile_definitions(${LIB} PRIVATE OSGL_EXPORTS=1)
endif()

install (
    TARGETS ${LIB}
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	COMPONENT Libraries
	)

install (
	FILES ${HDRS}
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/osgl
	COMPONENT Libraries
	)

install (
    DIRECTORY glad/include/glad glad/include/KHR glad/include/EGL
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	COMPONENT Libraries
    )

if (BUILD_OSGL_TESTS)
    add_subdirectory (tests)
endif()

