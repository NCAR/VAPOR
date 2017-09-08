file (GLOB EFFECT_FILES *.efc)

install (
	FILES ${EFFECT_FILES}
	DESTINATION ${INSTALL_SHARE_DIR}/shaders
	COMPONENT Share
	)

install (
	DIRECTORY includes main
	DESTINATION ${INSTALL_SHARE_DIR}/images
	COMPONENT Share
	)
