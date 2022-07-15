set(ALL_NOTEBOOKS "")

function(NOTEBOOK FILE OUT)
    include(CMakeParseArguments)
    cmake_parse_arguments(PARSE_ARGV 2
        "NOTEBOOK"
        ""
        ""
        ""
        )

    get_filename_component(FILE "${FILE}" REALPATH)

    add_custom_command(
        OUTPUT "${OUT}"
        DEPENDS "${FILE}"
        COMMAND cat "${FILE}" | jupytext --to ipynb > "${OUT}"
        )

    list(APPEND ALL_NOTEBOOKS "${OUT}")
    set(ALL_NOTEBOOKS "${ALL_NOTEBOOKS}" PARENT_SCOPE)
endfunction()


# copy(vapor ${PYTHON_API_DIR} EXCLUDE cmake.py __pycache__)
# copy(setup.py ${PYTHON_API_DIR})

find_program(JUPYTEXT "jupytext")
if (JUPYTEXT)
    set(EXAMPLE_NOTEBOOK_OUT_DIR "${PYTHON_API_DIR}/example_notebooks")
    file(MAKE_DIRECTORY ${EXAMPLE_NOTEBOOK_OUT_DIR})

    copy(example_utils.py ${EXAMPLE_NOTEBOOK_OUT_DIR})
    notebook(numpy_example.py ${EXAMPLE_NOTEBOOK_OUT_DIR}/numpy_example.ipynb)
    notebook(xarray_example.py ${EXAMPLE_NOTEBOOK_OUT_DIR}/xarray_example.ipynb)
    notebook(dataset_example.py ${EXAMPLE_NOTEBOOK_OUT_DIR}/dataset_example.ipynb)
    notebook(flow_example.py ${EXAMPLE_NOTEBOOK_OUT_DIR}/flow_example.ipynb)
    notebook(annotation_example.py ${EXAMPLE_NOTEBOOK_OUT_DIR}/annotation_example.ipynb)
    notebook(camera_example.py ${EXAMPLE_NOTEBOOK_OUT_DIR}/camera_example.ipynb)
    notebook(transfer_function_example.py ${EXAMPLE_NOTEBOOK_OUT_DIR}/transfer_function_example.ipynb)
    notebook(workflow_example.py ${EXAMPLE_NOTEBOOK_OUT_DIR}/workflow_example.ipynb)

    add_custom_target(
        example_notebooks
        ALL
        DEPENDS ${ALL_NOTEBOOKS}
    )
else()
    message(WARNING "jupytext not found")
    message(WARNING "Skipping generation of jupyter notebooks")
endif()


if (CONDA_BUILD)
    set(CONDA_SITE_PACKAGE_DIR "$ENV{SP_DIR}")
    install(
        DIRECTORY "${PYTHON_API_DIR}/examples"
		DESTINATION "${CONDA_SITE_PACKAGE_DIR}/vapor"
		COMPONENT Dependencies
	)
    if (JUPYTEXT)
        install(
            DIRECTORY "${EXAMPLE_NOTEBOOK_OUT_DIR}"
		    DESTINATION "${CONDA_SITE_PACKAGE_DIR}/vapor"
		    COMPONENT Dependencies
	    )
    endif()
endif()

