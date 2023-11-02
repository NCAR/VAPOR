if (NOT PROJECT_NAME)
	cmake_minimum_required (VERSION 3.10)
    project (VAPI)
	set (CMAKE_CXX_STANDARD 11)
endif ()

file (GLOB HDRS ./*.h)
file (GLOB SRCS ./*.cpp)
if (APPLE)
	file (GLOB OBJC_SRCS ./*.mm)
	list (APPEND SRCS ${OBJC_SRCS})
endif ()

list(FILTER SRCS EXCLUDE REGEX ".*main\\.cpp$")

add_library (vapi SHARED ${SRCS} ${HDRS})

target_link_libraries (
    vapi
	render
    ${OSGL}
)

if (APPLE)
	find_library (APPKIT AppKit)
	target_link_libraries (vapi ${APPKIT})
endif ()

# if (UNIX AND NOT APPLE)
# 	target_link_libraries (vapi OSMesa EGL)
# endif ()

file (GLOB GLContextLibFiles ./GLContext*.cpp ./GLContext*.h ./GLContext*.mm)
source_group (GLContextLib FILES ${GLContextLibFiles})

# add_executable (vapitest main.cpp)
# target_link_libraries (vapitest vapi ${Python_LIBRARIES})


file (
    COPY ${HDRS}
    DESTINATION ${CMAKE_BINARY_DIR}/include/vapor
    )
target_include_directories (vapi PUBLIC ${CMAKE_BINARY_DIR}/include)


install (
	TARGETS vapi
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)

install (
	FILES ${HDRS}
	DESTINATION ${INSTALL_INCLUDE_DIR}
	COMPONENT Libraries
	)

