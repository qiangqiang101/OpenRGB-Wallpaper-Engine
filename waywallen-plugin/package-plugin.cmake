file(REMOVE "${PLUGIN_ZIP_PATH}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar "cf" "${PLUGIN_ZIP_PATH}" --format=zip -- .
    WORKING_DIRECTORY "${PLUGIN_PACKAGE_DIR}"
    RESULT_VARIABLE _zip_result
)
if(NOT _zip_result EQUAL 0)
    message(FATAL_ERROR "Failed to create plugin zip")
endif()
