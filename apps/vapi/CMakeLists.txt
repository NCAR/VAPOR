if (NOT PROJECT_NAME)
	cmake_minimum_required (VERSION 3.10)
    project (VAPI)
	set (CMAKE_CXX_STANDARD 11)
endif ()

file (GLOB SRCS ./*.cpp ./*.h)
if (APPLE)
	file (GLOB OBJC_SRCS ./*.mm)
	list (APPEND SRCS ${OBJC_SRCS})
endif ()

set (GUI_SRCS )
list (APPEND SRCS ${GUI_SRCS})
source_group (temp_VaporGUI FILES ${GUI_SRCS})

add_executable (vapi ${SRCS})

target_link_libraries (
    vapi
	render
	${OPENGL_LIBRARIES} 
)

if (APPLE)
	find_library (APPKIT AppKit)
	target_link_libraries (vapi ${APPKIT})
endif ()

if (UNIX AND NOT APPLE)
	target_link_libraries (vapi OSMesa EGL)
endif ()


file (GLOB GLContextLibFiles ./GLContext*.cpp ./GLContext*.h ./GLContext*.mm)
source_group (GLContextLib FILES ${GLContextLibFiles})
