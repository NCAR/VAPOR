
set (TARGETS
	vaporversion
	vaporgui
	)

foreach (target ${TARGETS})
	message ("generating ${target}_launcher")
	set (TARGET_NAME ${target})
	configure_file (launcher.c ${target}_launcher.c)
	add_executable (
		${target}_launcher
		# EXCLUDE_FROM_ALL
		${target}_launcher.c
		)
	set_target_properties(
		${target}_launcher PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/launchers
		OUTPUT_NAME ${target}
		)
	install (
		TARGETS ${target}_launcher 
		DESTINATION ${INSTALL_LAUNCHER_DIR}
		COMPONENT Utilites
		)
endforeach (target)
