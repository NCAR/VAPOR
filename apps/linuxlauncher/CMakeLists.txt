set (TARGETS
	cf2vdc
	ncdf2wasp
	tiff2geotiff
	vaporversion
	vdccreate
	wasp2raw
	wrfvdccreate
	cfvdccreate
	raw2vdc
	vapor
	vdcdump
	waspcreate
	raw2wasp
	vdc2raw
	wasp2ncdf
	wrf2vdc
	)

append (LAUNCHER_TARGETS _launcher ${TARGETS})
set_property (GLOBAL PROPERTY LAUNCHER_TARGETS ${LAUNCHER_TARGETS})

foreach (target ${TARGETS})
	set (TARGET_NAME ${target})
	configure_file (launcher.c ${target}_launcher.c)
	add_executable (
		${target}_launcher
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
