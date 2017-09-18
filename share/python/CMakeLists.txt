set (PY_FILES
	plot1D.py
	pythonSystemStartup.py
	vapor_utils.py
	vapor_wrf.py
)

install (
	FILES ${PY_FILES}
	DESTINATION ${INSTALL_SHARE_DIR}/python
	COMPONENT Share
	)
